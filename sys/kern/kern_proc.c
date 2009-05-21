/*
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */


#include <sys/types.h>
#include <sys/utils.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/kmem.h>
#include <sys/vm.h>

static pid_t last_pid = 1;
list_t procs_list;
kmem_cache_t *proc_cache;
static bool find_this_pid(proc_t *p, pid_t pid);
static void proc_ctor(void *obj);
static void proc_dtor(void *obj);
proc_t proc0;


void
proc_ctor(void *obj)
{
    proc_t *proc = obj;
    proc->vm_space = kmem_alloc(sizeof(vm_space_t), KM_SLEEP);
    proc->p_cred = kmem_alloc(sizeof(pcred_t), KM_SLEEP);
}

void
proc_dtor(void *obj)
{
    proc_t *proc = obj;
    kmem_free(proc->p_cred);
    kmem_free(proc->vm_space);
}


/// Inicjalizuje obs�ug� proces�w.
void
proc_init(void)
{
    last_pid = INIT_PID;
    LIST_CREATE(&procs_list, proc_t, L_procs, FALSE);
    proc_cache = kmem_cache_create("proc", sizeof(proc_t), proc_ctor,
        proc_dtor);

    proc0.p_pid = 1;
    proc0.p_ppid = 0;
    proc0.p_cred = NULL;
    proc0.p_cred = kmem_alloc(sizeof(pcred_t), KM_SLEEP);
    proc0.p_cred->p_uid = 0;
    LIST_CREATE(&proc0.p_threads, thread_t, L_threads, FALSE);
    LIST_CREATE(&proc0.p_children, proc_t, L_children, FALSE);
    list_insert_head(&procs_list, &proc0);
    list_insert_head(&proc0.p_threads, curthread);
    curthread->thr_proc = &proc0;
    proc0.vm_space = kmem_alloc(sizeof(vm_space_t), KM_SLEEP);
    curthread->vm_space = proc0.vm_space;
    vm_space_create(proc0.vm_space, VM_SPACE_USER);
}

/**
 * Przydziela deskryptor procesu.
 */
proc_t *
proc_create(void)
{
    proc_t *new_p = kmem_cache_alloc(proc_cache, KM_SLEEP);
    KASSERT(new_p != NULL);

    new_p->p_pid = last_pid++;
    new_p->p_ppid = 0; // XXX: fork
    new_p->p_cred->p_uid = 0;  // XXX: fork

    LIST_CREATE(&new_p->p_threads, thread_t, L_threads, FALSE);
    LIST_CREATE(&new_p->p_children, proc_t, L_children, FALSE);
    list_insert_head(&procs_list, new_p);

    vm_space_create(new_p->vm_space, VM_SPACE_USER);
    return new_p;
}

/**
 * Niszczy proces
 * @param proc - proces do zniszczenia
 */
void
proc_destroy(proc_t *proc)
{
    thread_t *t_iter;
    proc_t *p_iter;
    proc_t *init = proc_find(INIT_PID);

    kprintf("%x - dlugosc\n", list_length(&proc->p_threads));

    /* niszczymy w�tki */
    while ( (t_iter = (thread_t *)list_extract_first(&(proc->p_threads))) )
        thread_destroy(t_iter);

    /* przepinamy dzieci */
    while ( (p_iter = (proc_t *)list_extract_first(&(proc->p_children))) )
    {
            // przepinamy dziecko pod INIT_PID
            proc_insert_child(init, p_iter);
    }

    kmem_free(proc->p_cred);
    kmem_cache_free(proc_cache, proc);
    return;
}

thread_t *
proc_create_thread(proc_t *proc, size_t stack_size, addr_t entry)
{
    thread_t *t = thread_create(THREAD_USER, entry, NULL);
    t->vm_space = proc->vm_space;
    t->thr_stack_size = stack_size;
    t->thr_kstack_size = THREAD_KSTACK_SIZE;
    t->thr_proc = proc;
    vm_space_create_stack(proc->vm_space, &t->thr_stack, stack_size);
    vm_space_create_stack(&vm_kspace, &t->thr_kstack, THREAD_KSTACK_SIZE);
    DEBUGF("user thread created");
    vm_space_print(t->vm_space);
    DEBUGF("   KSTACK  %p-%p (+%p)", t->thr_kstack, t->thr_kstack
        + t->thr_kstack_size, t->thr_kstack_size);
    return t;
}

/**
 * Sprawdza czy dany proces jest ZOMBIE
 */

bool
proc_is_zombie(proc_t *p)
{
    return (p->p_flags & PROC_ZOMBIE) ? TRUE : FALSE;
}

/**
 * Dodaje dziecko do procesu
 * @param proc - proces rodzic
 * @param child - proces dziecko
 */

void
proc_insert_child(proc_t *proc, proc_t *child)
{
    list_insert_tail(&(proc->p_children), child);
    child->p_ppid = proc->p_pid;
    return;
}

bool find_this_pid(proc_t *p, pid_t pid)
{
    return (p->p_pid == pid);
}

/**
 * Zwraca struktur� procesu dla wybranego identyfikatora.
 * @param pid identyfikator procesu
 *
 * Procedura zwraca struktur� procesu na podstawie identyfikatora, w przypadku
 * gdy identyfikatorowi nie odpowiada �aden proces zwracany jest NULL.
 */

proc_t *
proc_find(pid_t pid)
{
    return list_find(&procs_list, find_this_pid, pid);
}

/**
 * Sprawdza czy proces parent jest rodzicem procesu child
 */

bool
proc_is_parent(proc_t *parent, proc_t *child)
{
    return (child->p_ppid == parent->p_pid) ? TRUE : FALSE;
}

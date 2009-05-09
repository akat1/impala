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

pid_t last_pid;
list_t procs_list;
proc_t *curproc;

bool find_this_pid(proc_t *p, pid_t pid);

/// Inicjalizuje obs³ugê procesów.

void
proc_init(void)
{
    last_pid = INIT_PID;
    LIST_CREATE(&procs_list, proc_t, L_procs, FALSE);
    return;
}

/**
 * Przydziela deskryptor procesu.
 */

proc_t *
proc_create(void)
{
    proc_t *new_p = (proc_t *)kmem_alloc(sizeof(proc_t), KM_NOSLEEP);

    kprintf("%p\n",new_p);

    if ( new_p == NULL )
        panic("No free procs left");

    new_p->p_pid = last_pid++;
    new_p->p_ppid = 0; // XXX: fork
    new_p->p_cred->p_uid = 0;  // XXX: fork

    LIST_CREATE(&(new_p->p_threads), thread_t, L_threads, FALSE);
    LIST_CREATE(&(new_p->p_children), proc_t, L_children, FALSE);
    list_insert_head(&procs_list, new_p);
    
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

    /* niszczymy w±tki */
    while ( (t_iter = (thread_t *)list_extract_first(&(proc->p_threads))) )
        thread_destroy(t_iter);

    /* przepinamy dzieci */
    while ( (p_iter = (proc_t *)list_extract_first(&(proc->p_children))) )
    {
            // przepinamy dziecko pod INIT_PID 
            proc_insert_child(init, p_iter);
    }

    kmem_free(proc->p_cred);
    kmem_free(proc);

    return;
}

/**
 * Dodaje w±tek do procesu.
 * @param proc - proces
 * @param thread - watek
 */
void
proc_insert_thread(proc_t *proc, thread_t *thread)
{
    thread->thr_proc = proc;
    list_insert_head(&(proc->p_threads), thread);
    return;
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
 * Zwraca strukturê procesu dla wybranego identyfikatora.
 * @param pid identyfikator procesu
 *
 * Procedura zwraca strukturê procesu na podstawie identyfikatora, w przypadku
 * gdy identyfikatorowi nie odpowiada ¿aden proces zwracany jest NULL.
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

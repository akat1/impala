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
#include <sys/kernel.h>
#include <sys/vm.h>
#include <sys/termios.h>
#include <sys/device.h>
#include <sys/file.h>
#include <sys/ipc.h>

static pid_t last_pid = 0;
list_t procs_list;
kmem_cache_t *proc_cache;
static bool find_this_pid(proc_t *p, pid_t pid);
static void proc_ctor(void *obj);
static void proc_dtor(void *obj);
proc_t proc0;
proc_t *initproc;

///@todo synchronizacja na globalnych strukturach, jak lista procesów itp.

void
proc_ctor(void *obj)
{
    mem_zero(obj, sizeof(proc_t));
    proc_t *proc = obj;
    proc->vm_space = 0;
    proc->p_cred = kmem_zalloc(sizeof(pcred_t), KM_SLEEP);
    proc->p_fd = filetable_alloc();
    proc->p_cmd = NULL;
    proc->p_nice = PROC_NZERO;
    sleepq_init(&proc->p_waitq);
    mutex_init(&proc->p_mtx, MUTEX_NORMAL);
    LIST_CREATE(&proc->p_umtxs, mutex_t, L_umtxs, FALSE);
}

void
proc_dtor(void *obj)
{
    proc_t *proc = obj;
    if (proc->p_cmd) kmem_free((void*)proc->p_cmd);
    kmem_free(proc->p_cred);
    kmem_free(proc->vm_space);
    mutex_destroy(&proc->p_mtx);
    filetable_free(proc->p_fd);
}

int
proc_getinfos(int off, struct procinfo *tab, int n)
{
//    MUTEX_LOCK(&proc_mtx, "procinfo");
    int r = 0;
    int i;
    proc_t *p = list_head(&procs_list);
    for (i = 0; i < off && (p = list_next(&procs_list,p)); i++);
    if (i == off && p != NULL) {
        for (i = 0; i < n && p; i++, p = list_next(&procs_list,p)) {
            tab[i].pid = p->p_pid;
            tab[i].ppid = p->p_ppid;
            tab[i].nice = p->p_nice;
            tab[i].pri = p->p_pri;
            tab[i].threads = list_length(&p->p_threads);
            if (p->p_ctty)
                str_ncpy(tab[i].tty, p->p_ctty->t_dev->name, sizeof(tab[i].tty));
                else tab[i].tty[0] = 0;
            if (p->p_cmd)
                str_ncpy(tab[i].cmd, p->p_cmd, sizeof(tab[i].cmd));
                else tab[i].cmd[0] = 0;
        }
        r = i;
    }
//    mutex_unlock(&pro_mtx);
    return r;
}

/// Inicjalizuje obsługę procesów.
void
proc_init(void)
{
    last_pid = 0;
    LIST_CREATE(&procs_list, proc_t, L_procs, FALSE);
    proc_cache = kmem_cache_create("proc", sizeof(proc_t), proc_ctor,
        proc_dtor);

    proc0.p_pid = last_pid++;
    proc0.p_group = proc0.p_pid;
    proc0.p_ppid = 0;
    proc0.p_nice = PROC_NZERO;
    proc0.p_cred = NULL;
    proc0.p_cmd = str_dup(karg_get_name());
    LIST_CREATE(&proc0.p_threads, thread_t, L_pthreads, FALSE);
    LIST_CREATE(&proc0.p_children, proc_t, L_children, FALSE);

    list_insert_head(&procs_list, &proc0);
    list_insert_head(&proc0.p_threads, curthread);
    curthread->thr_proc = &proc0;
    proc0.vm_space = &vm_kspace;
    curthread->vm_space = proc0.vm_space;
    mutex_init(&proc0.p_mtx, MUTEX_NORMAL);
}

void
proc_exit(proc_t *p, int exit)
{
    thread_t *t = NULL;
    bool alive = FALSE;
    while ( (t = list_next(&p->p_threads, t)) ) {
        if (t != curthread)
            sched_exit(t);
        else
            alive = TRUE;
    }
    p->p_flags = PROC_ZOMBIE;
    p->p_status = exit;
    /* powiadom rodzica */
    proc_destroy(p);
    if(alive)
        sched_exit(curthread);
}

#include <machine/thread.h>
#include <machine/interrupt.h>


int
proc_fork(proc_t *p, proc_t **child)
{
    thread_t *t = curthread;
    proc_t *cp = proc_create();
    proc_insert_child(p, cp);
    proc_reset_vmspace(cp);
    vm_space_clone(cp->vm_space, p->vm_space);

    // tablica deskryptorów
    filetable_clone(cp->p_fd, p->p_fd);
    // CWD
    if (p->p_cmd) cp->p_cmd = str_dup(cp->p_cmd);
    cp->p_rootdir = p->p_rootdir;
    vref(cp->p_rootdir);
    cp->p_curdir = p->p_curdir;
    vref(cp->p_curdir);
    cp->p_session = p->p_session;
    cp->p_group = p->p_group;
    cp->p_ctty = p->p_ctty;
    cp->p_brk_addr = p->p_brk_addr;
    // Kopia IPC SystemV MSG
    // Reset clock

    thread_t *ct = proc_create_thread(cp, thread_get_pc(t));
    thread_fork(t, ct);
    *child = cp;
    sched_insert(ct);
    return 0;
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
    new_p->p_flags = PROC_NEW;
    new_p->p_cred->p_uid = 0;  // XXX: fork
    new_p->p_ctty = NULL; //zaczynamy bez terminala kontrolującego
    new_p->p_group = -1;
    new_p->p_session = -1;
    new_p->p_rootdir = rootvnode;
    new_p->p_curdir = rootvnode;
    KASSERT(rootvnode!=NULL);
    LIST_CREATE(&new_p->p_threads, thread_t, L_pthreads, FALSE);
    LIST_CREATE(&new_p->p_children, proc_t, L_children, FALSE);
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
    proc_t *p;

    list_remove(&procs_list, proc);
    signal_send(proc_find(proc->p_ppid), SIGCHLD);
    sleepq_wakeup(&(proc_find(proc->p_ppid)->p_waitq));

    proc_destroy_threads(proc);
    if (proc->vm_space) {
        vm_space_destroy(proc->vm_space);
        proc->vm_space = 0;
    }
    while ( (p = list_extract_first(&(proc->p_children))) )
    {
        proc_insert_child(initproc, p);
    }
    filetable_close(proc->p_fd);
    ///proszę nie usuwać rzeczy utworzonych w ctor ;D
 
    return;
}

/// Finalne wyrzucenie procesu z proc_cache

void
proc_delete(proc_t *proc)
{
    kmem_cache_free(proc_cache, proc);
}


void
proc_destroy_threads(proc_t *proc)
{
    MUTEX_LOCK(&proc->p_mtx, "proc");
    mutex_t *m;

    while ( (m = list_extract_first(&(proc->p_umtxs)) )) {
        mutex_destroy(m);
    }

    thread_t *t;
    while ( (t = list_extract_first(&(proc->p_threads))) )
        if (t != curthread) thread_destroy(t);
    mutex_unlock(&proc->p_mtx);
}


void
proc_destroy_thread(proc_t *proc, thread_t *ut)
{
    MUTEX_LOCK(&proc->p_mtx, "proc");
    list_remove(&proc->p_threads, ut);
    mutex_unlock(&proc->p_mtx);
    if (ut==curthread) {
        thread_exit_last(ut);
    } else {
        thread_destroy(ut);
    }
}

void
proc_reset_vmspace(proc_t *p)
{
    MUTEX_LOCK(&p->p_mtx, "proc");
    if (p->vm_space) {
        vm_space_destroy(p->vm_space);
    } else {
        p->vm_space = kmem_alloc(sizeof(vm_space_t), KM_SLEEP);
    }
    vm_space_create(p->vm_space, VM_SPACE_USER);
//    p->p_brk_addr = p->vm_space->seg_data->end; <- to nie może tu być
    mutex_unlock(&p->p_mtx);
}


thread_t *
proc_create_thread(proc_t *proc, uintptr_t entry)
{
    MUTEX_LOCK(&proc->p_mtx, "proc");
    thread_t *t = thread_create(THREAD_USER, 0, NULL);
    t->vm_space = proc->vm_space;
    t->thr_entry_point = (void*)entry;
    t->thr_entry_arg = 0;
    t->thr_kstack_size = THREAD_KSTACK_SIZE;
    t->thr_proc = proc;
    t->thr_joiner = kmem_alloc(sizeof(sleepq_t), KM_SLEEP);
    sleepq_init(t->thr_joiner);
    list_insert_tail(&proc->p_threads, t);
    mutex_unlock(&proc->p_mtx);
    return t;
}


bool
proc_has_thread(proc_t *p, thread_t *t)
{
    MUTEX_LOCK(&p->p_mtx, "proc");
    bool x = list_is_member(&p->p_threads, t);
    mutex_unlock(&p->p_mtx);
    return x;
}

bool
proc_has_mutex(proc_t *p, mutex_t *m)
{
    MUTEX_LOCK(&p->p_mtx, "proc");
    bool x = list_is_member(&p->p_umtxs, m);
    mutex_unlock(&p->p_mtx);
    return x;
}

mutex_t *
proc_create_mutex(proc_t *p)
{
    MUTEX_LOCK(&p->p_mtx, "proc");
    mutex_t *umtx = kmem_alloc(sizeof(*umtx), KM_SLEEP);
    mutex_init(umtx, MUTEX_CONDVAR|MUTEX_USER);
    list_insert_tail(&p->p_umtxs, umtx);
    mutex_unlock(&p->p_mtx);
    return umtx;
}

int
proc_destroy_mutex(proc_t *p, mutex_t *um)
{
    // Sprawdzamy czy użytkownik nie wpadł´na pomysl zabić nam
    // używaną blokadę. Ustawiamy odpowiedni SPL aby nie dopuścić
    // że pomiędzy trylock() a usunięciem będzie zmiana kontekstu
    // i użytkownik zamknie blokadę.
    int s = splsoftclock();
    if (!mutex_trylock(um)) {
        splx(s);
        return -EINVAL;
    }
    mutex_destroy(um);
    list_remove(&p->p_umtxs, um);
    splx(s);
    return 0;
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
    MUTEX_LOCK(&proc->p_mtx, "proc");
    list_insert_tail(&(proc->p_children), child);
    child->p_ppid = proc->p_pid;
    mutex_unlock(&proc->p_mtx);
    return;
}

bool
find_this_pid(proc_t *p, pid_t pid)
{
    return (p->p_pid == pid);
}

/**
 * Zwraca strukturę procesu dla wybranego identyfikatora.
 * @param pid identyfikator procesu
 *
 * Procedura zwraca strukturę procesu na podstawie identyfikatora, w przypadku
 * gdy identyfikatorowi nie odpowiada żaden proces zwracany jest NULL.
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

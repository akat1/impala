/* Impala Operating System
 *
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

#ifndef __SYS_THREAD_H
#define __SYS_THREAD_H

#include <machine/thread.h>
#include <machine/atomic.h>
#include <sys/list.h>
#include <sys/sched.h>

enum {
    MAX_THREAD = 0x10,
    THREAD_STACK_SIZE = 0x2000
};

/// wiruj�cy zamek.
struct spinlock {
    volatile int    _dlock;
};

/// w�tek procesora.
struct thread {
    /// stan procesora.
    thread_context  thr_context;
    int             thr_tid;
    /// adres procedury wej�ciowej
    addr_t          thr_entry_point;
    /// adres argumenty procedury wej�ciowej
    addr_t          thr_entry_arg;
    /// poziom uprzywilejowania
    int             thr_priv;
    /// opcje
    int             thr_flags;
    uint            thr_wakeup_time;
    /// stos (tymczasowo tutaj)
    char            thr_stack[THREAD_STACK_SIZE];
    char            thr_kstack[THREAD_STACK_SIZE];
    /// proces, do kt�rego w�tek przynale�y
    proc_t         *thr_proc;
    /// w�ze� kolejki planisty
    list_node_t     L_run_queue;
    /// w�ze� listy w�tk�w
    list_node_t     L_threads;
    /// w�ze� listy w�tk�w oczekuj�cych
    list_node_t     L_wait;
};

/// zamek typu mutex.
struct mutex {
    /// w�tek b�d�cy w�a�cicielem zamka.
    thread_t     *mtx_owner;
    /// stan zamka.
    int           mtx_locked;
    /// opcje.
    int           mtx_flags;
    /// pomocniczy wiruj�cy zamek.
    spinlock_t    mtx_slock;
    /// lista w�tk�w oczekuj�cych na wej�cie
    list_t        mtx_locking; // thread_t.L_wait
    /// lista w�tk�w oczekuj�cych na poinformowanie
    list_t        mtx_waiting; // thread_t.L_wait
};


struct semaph {
    mutex_t         mtx;
    int             count;
};

/// wsp�biezna kolejka
struct cqueue {
    /// zamek
    mutex_t     q_mtx;
    /// lista danych
    list_t      q_data;
};


enum THREAD_FLAGS {
    THREAD_NEW       = 1 << 0, // in-creating
    THREAD_ZOMBIE    = 1 << 1, // in-destroying
    THREAD_RUN       = 1 << 2, // are running
    THREAD_SYSCALL   = 1 << 3, // are in syscall handler
    THREAD_SLEEP     = 1 << 4, // sleeped
    THREAD_INPROC    = 1 << 5, // connected to user process
    THREAD_INRUNQ    = 1 << 6, // are in run-queue
    THREAD_KERNEL    = 1 << 7  // kernel thread
};

enum {
    MUTEX_NORMAL     = 0,
    MUTEX_CONDVAR    = 1 << 0,
    MUTEX_WAKEUP_ONE = 1 << 1,
    MUTEX_WAKEUP_ALL = 1 << 2
};


enum {
    MUTEX_LOCKED,
    MUTEX_UNLOCKED
};



enum {
    SPINLOCK_UNLOCK,
    SPINLOCK_LOCK
};

#ifdef __KERNEL
extern thread_t *curthread;     // nie lepiej curthread ?
extern thread_t *thread_idle;
extern list_t threads_list;     // thread_t.L_threads

void thread_init(void);
thread_t *thread_create(int priv, addr_t entry, addr_t arg);
void thread_destroy(thread_t *t);

// do wywalenia
void thread_suspend(thread_t *t);

void mutex_init(mutex_t *m, int flags);
void mutex_lock(mutex_t *m);
bool mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);
void mutex_destroy(mutex_t *m);
void mutex_wait(mutex_t *m);
void mutex_wakeup(mutex_t *m);
void mutex_wakeup_all(mutex_t *m);

void cqueue_init(cqueue_t *m, int off);
void cqueue_insert(cqueue_t *m, void *d);
void *cqueue_extract(cqueue_t *m);
void cqueue_shutdown(cqueue_t *m);

void semaph_init(semaph_t *s);
void semaph_post(semaph_t *s);
void semaph_wait(semaph_t *s);
void semaph_destroy(semaph_t *s);


/// Inicjalizuje wiruj�cy zamek.
static inline void
spinlock_init(spinlock_t *sp)
{
    sp->_dlock = SPINLOCK_UNLOCK;
}

/// Zamyka zamek.
static inline void
spinlock_lock(spinlock_t *sp)
{
    while (atomic_change_int(&sp->_dlock, SPINLOCK_LOCK) == SPINLOCK_LOCK) {
        sched_yield();
    }
}

/// Odblokowuje zamek.
static inline void
spinlock_unlock(spinlock_t *sp)
{
      sp->_dlock = SPINLOCK_UNLOCK;
}

/// Pr�buje zamkn�� zamek.
static inline bool
spinlock_trylock(spinlock_t *sp)
{
    return atomic_change_int(&sp->_dlock, SPINLOCK_LOCK) == SPINLOCK_UNLOCK;
}

static inline void
spinlock_destroy(spinlock_t *sp)
{
}

#endif
#endif


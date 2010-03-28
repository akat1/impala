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

#ifdef __KERNEL
#include <sys/list.h>
#include <sys/sched.h>
#include <sys/signal.h>
#include <machine/thread.h>
#include <machine/atomic.h>

enum {
    THREAD_STACK_SIZE = 0x10000,  ///< domyślny rozmiar stosu
    THREAD_KSTACK_SIZE = 0x8000   ///< domyślny rozmiar stosu alt.
};

/// wirujący zamek.
struct spinlock {
    volatile int    _dlock;
};

/// zamek typu mutex.
struct mutex {
    thread_t     *mtx_owner;   ///< wątek będący właścicielem zamka.
    int           mtx_locked;  ///< stan zamka.
    int           mtx_flags;   ///< opcje.
    spinlock_t    mtx_slock;   ///< pomocniczy wirujący zamek.
    list_t        mtx_locking; 
    list_t        mtx_waiting; 
    list_node_t   L_umtxs;
};

typedef struct wdescr wdescr_t;
struct wdescr {
    int              line;
    const char      *file;
    const char      *func;
    const char      *descr;
};

/// wątek procesora.
struct thread {
    thread_context  thr_context;    ///< kontekst
    signal_context *thr_sigcontext; ///< stos kontekstów dla sygnałów
    addr_t          thr_entry_point;///< adres procedury wejściowej
    addr_t          thr_entry_arg;  ///< adres argumenty procedury wejściowej
    int             thr_flags;      ///< opcje
    uint            thr_wakeup_time;///< kiedy obudzić uśpiony wątek
    vm_space_t     *vm_space;       ///< przestrzen, w ktorej jest watek.
    addr_t          thr_stack;      ///< stos
    size_t          thr_stack_size; ///< rozmiar stosu
    addr_t          thr_kstack;     ///< stos dla jadra
    size_t          thr_kstack_size;///< rozmiar stosu dla jadra
    proc_t         *thr_proc;       ///< proces, do którego wątek przynależy
    sleepq_t       *thr_sleepq;     ///< kolejka w której śpi wątek
    wdescr_t        thr_wdescr;     ///<
    mutex_t         thr_mtx;        ///< do synchronizacji
    bool            thr_cancel;     ///< zgłoszenie anulowania wątku
    sigset_t        thr_sigblock;   ///< blokowane sygnały
    sleepq_t       *thr_joiner;     ///< pomocnik dla join'a
    list_node_t     L_run_queue;    ///< węzeł kolejki procesów gotowych do uruchomienia
    list_node_t     L_sched_queue;  ///< wezeł kolejki planisty
    list_node_t     L_threads;      ///< węzeł listy wątków
    list_node_t     L_pthreads;     ///< węzeł listy wątków w procesie.
    list_node_t     L_wait;         ///< węzeł listy wątków oczekujących
};

#define THREAD_SET_WDESCR(t,fl,fn,l,d)\
    do {\
        (t)->thr_wdescr.line = l;\
        (t)->thr_wdescr.file = fl;\
        (t)->thr_wdescr.func = fn;\
        (t)->thr_wdescr.descr = d;\
    } while (0)

/// śpiąca kolejka
struct sleepq {
    mutex_t     sq_mtx;
    list_t      sq_waiting;
};

enum {
    SLEEPQ_INTR = (1<<0)
};

struct semaph {
    mutex_t         mtx;
    int             count;
};

/// współbiezna kolejka
struct cqueue {
    mutex_t     q_mtx;     ///< zamek
    list_t      q_data;    ///< lista danych
};


enum THREAD_FLAGS {
    THREAD_NEW       = 1 << 0, //< w trakcie tworzenia
    THREAD_ZOMBIE    = 1 << 1, //< zombie
    THREAD_RUN       = 1 << 2, //< działa
    THREAD_SYSCALL   = 1 << 3, //< jest w obsłudze wywołania
    THREAD_SLEEP     = 1 << 4, //< uśpiony
    THREAD_INTRPT    = 1 << 5, //< przerwano spanie
    THREAD_INRUNQ    = 1 << 6, //<
    THREAD_USER      = 1 << 7, //< wątek użytkownika
    THREAD_SLEEPQ    = 1 << 8  //< uśpiony przez sleepq
};

enum {
    MUTEX_NORMAL     = 0,
    MUTEX_CONDVAR    = 1 << 0,
    MUTEX_WAKEUP_ONE = 1 << 1,
    MUTEX_WAKEUP_ALL = 1 << 2,
    MUTEX_USER       = 1 << 3
};

enum {
    MUTEX_LOCKED,
    MUTEX_UNLOCKED
};

enum {
    SPINLOCK_UNLOCK,
    SPINLOCK_LOCK
};

extern thread_t * volatile curthread;
extern thread_t *thread_idle;
extern list_t threads_list; 

void thread_init(void);
thread_t *thread_create(int space, addr_t entry, addr_t arg);
void thread_destroy(thread_t *t);
void thread_exit_last(thread_t *t);
void thread_clone(thread_t *dst, thread_t *src);
void thread_suspend(thread_t *t); // <- do wywalenia
uintptr_t thread_get_pc(thread_t *t);
void thread_fork(thread_t *t, thread_t *ct);
void thread_join(thread_t *t);

void mutex_init(mutex_t *m, int flags);
void mutex_lock(mutex_t *m, const char *, const char *, int , const char *);
#define MUTEX_LOCK(m,d) mutex_lock((m),__FILE__,__func__,__LINE__,(d))
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

void sleepq_init(sleepq_t *q);
void sleepq_destroy(sleepq_t *q);
#define SLEEPQ_WAIT(q,d) sleepq_wait(q, __FILE__, __func__, __LINE__, d)
void sleepq_wait(sleepq_t *q, const char *fl, const char *fn, int l, const char *d);
void sleepq_wait_ms(sleepq_t *q, uint ms);
int sleepq_wait_i(sleepq_t *q);
void sleepq_wakeup(sleepq_t *q);
void sleepq_intrpt(thread_t *td);

/// Inicjalizuje wirujący zamek.
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

/// Próbuje zamknąć zamek.
static inline bool
spinlock_trylock(spinlock_t *sp)
{
    return atomic_change_int(&sp->_dlock, SPINLOCK_LOCK) == SPINLOCK_UNLOCK;
}

static inline void
spinlock_destroy(spinlock_t *sp)
{
}

extern size_t thread_stack_size;
extern size_t thread_kstack_size;

#else
tid_t thr_create(void *entry, void *stackaddr, size_t stacksize,  void *arg);
int thr_exit(void);
int thr_join(tid_t tid);
int thr_cancel(tid_t tid);
tid_t thr_getid(void);
void* thr_getarg(void);

mid_t thr_mtx_create(void);
int thr_mtx_destroy(mid_t);
int thr_mtx_lock(mid_t);
int thr_mtx_unlock(mid_t);
int thr_mtx_trylock(mid_t);
int thr_mtx_wait(mid_t);
int thr_mtx_wakeup(mid_t);
int thr_mtx_wakeup_all(mid_t);

#endif

#endif


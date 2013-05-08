/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/utils.h>
#include <sys/kmem.h>
#include <sys/kargs.h>
#include <sys/string.h>
#include <sys/vm.h>
#include <machine/interrupt.h>

thread_t * volatile curthread;
thread_t *thread_idle;

list_t threads_list;

static kmem_cache_t *thread_cache;

static void thread_ctor(void *_thr);
static void thread_dtor(void *_thr);
size_t thread_stack_size = THREAD_STACK_SIZE;
size_t thread_kstack_size = THREAD_KSTACK_SIZE;

/*=============================================================================
 * Obsluga watkow.
 */


void
thread_ctor(void *_thr)
{
        mem_zero(_thr, sizeof(thread_t));
        thread_t *t = _thr;
        mutex_init(&t->thr_mtx, MUTEX_NORMAL);
        t->thr_joiner = 0;
}

void
thread_dtor(void *_thr)
{
        thread_t *t = _thr;
        mutex_destroy(&t->thr_mtx);
}

/// Inicjalizuje obsługę wątków.
void
thread_init()
{
    list_create(&threads_list, offsetof(thread_t, L_threads), FALSE);
    thread_cache = kmem_cache_create("thread", sizeof(thread_t),
        thread_ctor, thread_dtor);
    curthread = thread_create(0, 0, 0);
    curthread->thr_flags = THREAD_RUN;
    curthread->vm_space = &vm_kspace;
    mem_zero(&curthread->thr_wdescr, sizeof(wdescr_t));
    karg_get_i("stacksize", (int*)&thread_stack_size);
    karg_get_i("kstacksize", (int*)&thread_kstack_size);
}

/**
 * Przydziela deskryptor wątku.
 * @param type Poziom uprzywilejowania.
 * @param entry Adres procedury wejściowej.
 * @param arg Adres argumentu przekazywany do procedury wejściowej.
 *
 * Procedura przydziela ogólny deskryptor wątku. Wątek przydzielony
 * w ten sposób znajduje się w stanie surowym.
 */
thread_t *
thread_create(int type, addr_t entry, addr_t arg)
{
    thread_t *free_thr = kmem_cache_alloc(thread_cache, KM_SLEEP);
    if (free_thr) {
        thread_t *t = free_thr;
        t->thr_flags = THREAD_NEW | type;
        t->thr_entry_point = entry;
        t->thr_entry_arg = arg;
        t->thr_wakeup_time = 0;
        t->vm_space = NULL;
        t->thr_sleepq = NULL;
        list_insert_tail(&threads_list, t);

        vm_space_create_stack(&vm_kspace, &t->thr_kstack, thread_kstack_size);
        t->thr_kstack_size = thread_kstack_size;
        thread_context_init(t, &t->thr_context);
        return t;
    } else {
        kprintf("ERROR: no free threads!\n");
        return NULL;
    }
}

//na potrzeby zamknięcia ostatniego wątku procesu który robi exec
void thread_exit_last(thread_t *t)
{
    t->thr_flags |= THREAD_ZOMBIE;
    if (t->thr_joiner) {
        sleepq_wakeup(t->thr_joiner);
        sleepq_destroy(t->thr_joiner);
        t->thr_joiner = 0;
    }
    sched_exit_1(t);
    kmem_cache_free(thread_cache, t);
    //ta procedura nie wymaga już poprawnej zawartości struktury t
    sched_exit_2(t); 
    panic("thread_exit_last() - should not be here");
}

void
thread_destroy(thread_t *t)
{
    t->thr_flags |= THREAD_ZOMBIE;
    list_remove(&threads_list, t);
    if (t != curthread) {
        //bieżącego wątku nie możemy szybciej zwolnić, bo jest w runq
        // a ponowne przydzielenie tego samego adresu na nowy wątek narobiło by
        // w takiej sytuacji kłopotów
        if (t->thr_sleepq) {
            // jak sobie gdzięś śpi to go przed zabójstwem budzimy
            // inaczej będzie on w liście śpiącej kolejki, i po co ? ;]
            sleepq_intrpt(t);
        }
        if (t->thr_joiner) {
            sleepq_wakeup(t->thr_joiner);
            sleepq_destroy(t->thr_joiner);
            t->thr_joiner = 0;
        }
        sched_exit(t); //to nie aktualny wątek, więc możemy tak
        kmem_cache_free(thread_cache, t);
        return;
    }
    return;
}

/*=============================================================================
 * Obsluga mutexow. (FIFO).
 */

static void _mutex_wakeup(mutex_t *m);

/// Inicjalizuje deskryptor zamka mutex.
void
mutex_init(mutex_t *m, int flags)
{
    mem_zero(m, sizeof(mutex_t));
    m->mtx_flags = flags;
    m->mtx_locked = MUTEX_UNLOCKED;
    spinlock_init(&m->mtx_slock);
    list_create(&m->mtx_locking, offsetof(thread_t, L_wait), FALSE);
    if (flags & MUTEX_CONDVAR) {
        list_create(&m->mtx_waiting, offsetof(thread_t, L_wait), FALSE);
    }
}

/// Niszczy zamek.
void
mutex_destroy(mutex_t *m)
{
    KASSERT( m->mtx_flags & MUTEX_USER || list_length(&m->mtx_locking) == 0 );
    if (m->mtx_flags & MUTEX_CONDVAR)
        KASSERT( m->mtx_flags & MUTEX_USER || list_length(&m->mtx_waiting) == 0 );
    spinlock_destroy(&m->mtx_slock);
}

/**
 * Zamyka zamek.
 * @param m zamek do zamknięcia.
 *
 * W przypadku gdy zamek jest zajęty aktualny wątek
 * zostaje uśpiony.
 * W celu zapewnienia niegłodzenia implementowana jest strategia
 * wykorzystująca kolejkę FIFO.
 */

void
mutex_lock(mutex_t *m, const char *file, const char *func, int line,
    const char *descr)
{
//    KASSERT(CIPL==0);
    spinlock_lock(&m->mtx_slock);
    if ( atomic_change_int(&m->mtx_locked, MUTEX_LOCKED) == MUTEX_UNLOCKED) {
        m->mtx_owner = curthread;
        spinlock_unlock(&m->mtx_slock);
    } else {
        list_insert_tail(&m->mtx_locking, curthread);
        int x = spltty();
        spinlock_unlock(&m->mtx_slock);
        sched_wait(file,func,line,descr);
        splx(x);
    }
}

/**
 * Odblokowuje zamek.
 * @param m zamek.
 *
 */

void
mutex_unlock(mutex_t *m)
{
    thread_t *n;

    spinlock_lock(&m->mtx_slock);
    _mutex_wakeup(m);
    m->mtx_owner = NULL;
    n = list_extract_first(&m->mtx_locking);
    if (n) {
        m->mtx_owner = n;
        sched_wakeup(n);
    } else {
        m->mtx_locked = MUTEX_UNLOCKED;
    }
    spinlock_unlock(&m->mtx_slock);
}

/**
 * Budzenie wątków oczekujących na sygnał.
 */

void
_mutex_wakeup(mutex_t *m)
{
    int l = 0;
    if (m->mtx_flags & MUTEX_WAKEUP_ALL) {
        l = list_length(&m->mtx_waiting);
    } else
    if (m->mtx_flags & MUTEX_WAKEUP_ONE) {
        l = (list_length(&m->mtx_waiting)==0)? 0 : 1;
    }
    if (l)
    for (int i = 0; i < l; i++) {
        thread_t *n = list_extract_first(&m->mtx_waiting);
           list_insert_tail(&m->mtx_locking, n);
    }
    m->mtx_flags &= ~(MUTEX_WAKEUP_ONE|MUTEX_WAKEUP_ALL);
}

/**
 * Próbuje zamknąć zamek.
 * @param m zamek.
 * @return Zwraca prawdę wtedy i tylko wtedy, gdy udało się zamknąć zamek.
 *         w przeciwnym wypadku zwracany jest fałsz.
 */

bool
mutex_trylock(mutex_t *m)
{
    if (atomic_change_int(&m->mtx_locked, MUTEX_LOCKED) == MUTEX_UNLOCKED) {
        m->mtx_owner = curthread;
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * Oczekuje na danym zamku na sygnał.
 * @param m zamknięty zamek.
 *
 * Jeżeli wątek jest właścicielem zamku to może oczekiwać na nim sygnał.
 * Procedura wychodzi z sekcji krytycznej, a następnie usypia wątek.
 * Uśpiony wątek jest dodawany do listy wątków oczekujących na sygnał.
 *
 * Gdy wątek zostanie obudzony to zamek zostanie automatycznie mu
 * przydzielony (powróci do swojej sekcji krytycznej).
 */

void
mutex_wait(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
    list_insert_tail(&m->mtx_waiting, curthread);
    spinlock_unlock(&m->mtx_slock);
    sched_unlock_and_wait(m);
}

/**
 * Budzi jeden wątek oczekujący na sygnał.
 * @param m zamek.
 *
 * Jeżeli wątek jest właścicielem zamka, to może obudzić oczekujący sygnału
 * na tym zamku wątek. Procedura zaznacza informację, że przy odblokowaniu
 * zamku należy przenieść jeden wątek oczekujący na sygnał do listy wątków
 * chcących wejść do sekcji krytycznej.
 */

void
mutex_wakeup(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
    m->mtx_flags |= MUTEX_WAKEUP_ONE;
    spinlock_unlock(&m->mtx_slock);
}

/// Budzi wszystkie wątki oczekujące na sygnał.
void
mutex_wakeup_all(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
    m->mtx_flags |= MUTEX_WAKEUP_ALL;
    spinlock_unlock(&m->mtx_slock);
}

/*=============================================================================
 * Obsluga kolejek
 */


/**
 * Inicjalizuje współbieżną kolejkę.
 * @param q wskaźnik do deskryptora kolejki.
 * @param off przesunięcie uchwytu dla listy.
 */

void
cqueue_init(cqueue_t *q, int off)
{
    mutex_init(&q->q_mtx, MUTEX_CONDVAR);
    list_create(&q->q_data, off, FALSE);
}

/**
 * Wyłącza wspołbiezną kolejkę.
 * @param q kolejka.
 *
 * Niepozwala wątkom spać w oczekiwaniu na kolejne dane. Użyteczne
 * przy kończeniu pracy z daną kolejką. NIEZAIMPLEMENTOWANE.
 */
void
cqueue_shutdown(cqueue_t *q)
{
    mutex_destroy(&q->q_mtx);
}

/**
 * Wrzuca wskaźnik w kolejkę.
 * @param q kolejka
 * @param d wskaźnik do zakolejkowania.
 *
 * Procedura po zakolejkowaniu wskaźnika budzi jeden
 * z wątków oczekujących na dane.
 */
void
cqueue_insert(cqueue_t *q, void *d)
{
    MUTEX_LOCK(&q->q_mtx, "cqueue");
    list_insert_tail(&q->q_data, d);
    mutex_wakeup(&q->q_mtx);
    mutex_unlock(&q->q_mtx);
}

/**
 * Pobranie z kolejki wskaźnika.
 * @param q kolejka
 *
 * Procedura usypia wątek, gdy kolejka jest pusta a kolejka
 * nie zotała włączona. W przeciwnym wypadku zwraca NULL.
 */

void*
cqueue_extract(cqueue_t *q)
{
    void *p;
    MUTEX_LOCK(&q->q_mtx, "cqueue");
    while ( (p = list_extract_first(&q->q_data)) == NULL ) {
        mutex_wait(&q->q_mtx);
    }
    mutex_unlock(&q->q_mtx);
    return p;
}

/*============================================================================
 * Semafory
 */

void
semaph_init(semaph_t *sem)
{
    mutex_init(&sem->mtx, MUTEX_CONDVAR);
    sem->count = 0;
}

void
semaph_post(semaph_t *sem)
{
    MUTEX_LOCK(&sem->mtx, "semaph");
    sem->count++;
    mutex_wakeup(&sem->mtx);
    mutex_unlock(&sem->mtx);
}

void
semaph_wait(semaph_t *sem)
{
    MUTEX_LOCK(&sem->mtx, "semaph");
    if (sem->count == 0) {
        mutex_wait(&sem->mtx);
    }
    sem->count--;
    mutex_unlock(&sem->mtx);
}

void
semaph_destroy(semaph_t *sem)
{
    mutex_destroy(&sem->mtx);
}

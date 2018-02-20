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
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/clock.h>
#include <sys/utils.h>
#include <sys/kargs.h>
#include <sys/string.h>
#include <sys/signal.h>
#include <machine/interrupt.h>

/// Kwant czasu przydzialny programom, w tyknięciach zegara.
int sched_quantum;


/// Kolejka programów gotowych do uruchomienia
static list_t run_queue;

bool wantSched;
bool rescheduled;

static int end_ticks_roundrobin;
static int end_ticks_reschedule;

/// Zamek zabezpieczający sekcje krytyczne planisty.
static spinlock_t sprq;


static inline thread_t * select_next_thread(void);
static void __sched_yield(void);
static inline void _sched_wakeup(thread_t *n);

/*
 * Planista oparty na koncepcji szeregowania procesów z systemów 4.3BSD oraz
 * SVR3 - opis znajduje się w książce Jądro Systemu UNIX, nowe horyzony - Uresh
 * Vahalia - WNT
 */

list_t *__queue(int pri);
void __resched(void);


/// pierwsza niepsuta kolejka
int first_not_empty;
static list_t sched_queue[SCHED_NQ];

/// Procedura zwraca kolejkę na podstawie priorytetu
list_t *
__queue(int pri)
{
    return &sched_queue[pri/SCHED_PQ];
}

/// Procedura rozrzuca procesy po odpowiednich kolejkach
void
__resched(void)
{
    thread_t *t;
    int len = list_length(&run_queue);

    first_not_empty = SCHED_NQ-1;
 
    for ( int i = 0 ; i < SCHED_NQ ; ++i )
        list_create(&sched_queue[i], offsetof(thread_t, L_sched_queue), TRUE);
   
    t = list_head(&run_queue);
    for( int i = 0 ; i < len ; i++, t = list_next(&run_queue, t) ) {
        if ( ISSET(t->thr_flags, THREAD_USER)
            && ISSET(t->thr_flags, THREAD_RUN) ) {
            /* Wyliczamy na nowo priorytet - oryginalnie wyliczano na podstawie
             * wzoru PUSER + 2 * nice + decay(cpu) - w SVR4 decay(cpy) było
             * równe dzieleniu przez 4 w BSD było to dzielenie przez
             * średnie obciążenie systemu */
            t->thr_proc->p_pri = 2 * t->thr_proc->p_nice +
                (t->thr_proc->p_ucpu)/2;
            list_insert_head(__queue(t->thr_proc->p_pri), t);
            if ( first_not_empty > t->thr_proc->p_pri/SCHED_PQ )
                first_not_empty = t->thr_proc->p_pri/SCHED_PQ;
        }
    }

    t = list_head(&run_queue);
    for(int i = 0 ; i < len ; i++, t = list_next(&run_queue, t) ) {
        if ( !(ISSET(t->thr_flags, THREAD_USER))
            || ISUNSET(t->thr_flags, THREAD_RUN) )
            list_insert_head(&sched_queue[first_not_empty], t);
    }

    end_ticks_reschedule = clock_ticks + 
        SCHED_RESCHEDULE * sched_quantum;

    rescheduled = TRUE;

    return;
}

/// Procedura inicjująca program planisty.
void
sched_init()
{
    sched_quantum = 5;
    karg_get_i("sched_quantum", &sched_quantum);
    end_ticks_reschedule = clock_ticks + sched_quantum;
    list_create(&run_queue, offsetof(thread_t, L_run_queue), TRUE);
    spinlock_init(&sprq);
    sched_insert(curthread);
    __resched();
}

/**
 * Podprogram planisty.
 *
 * Procedura jest uruchamiana przez program obsługi przerwania
 * zegara. Odlicza odpowiedni kwant czasu i wyzwala zmiane kontekstu.
 */

void
sched_action()
{
    if (clock_ticks >= end_ticks_roundrobin && !wantSched) {
        wantSched=TRUE;
    }
}

/**
 * Pomocnicza procedura zmieniająca kontekst.
 *
 * Powinna być uruchamina tylko wewnątrz sekcji krytycznych
 * chronionych przez wirujący zamek sprq. Jej zadanie to wybranie
 * kolejnego wątku, wyjście z sekcji krytycznej i zmiana kontekstu.
 *
 * Stan na wej: przerwania włączone, CIPL == IPL_SOFTCLOCK
 */
void
__sched_yield()
{
    thread_t *n = select_next_thread();
    spinlock_unlock(&sprq); //nikt nam nie zablokuje, CIPL == IPL_SOFTCLOCK
    if (n == curthread) {
        return; // jednak nie zmieniamy
    }
    end_ticks_roundrobin = clock_ticks + sched_quantum;
    wantSched = FALSE;
    irq_disable();
    thread_switch(n, curthread);
    if ( curthread->thr_proc )
        curthread->thr_proc->p_ucpu = SCHED_UCPU_MAX;
    irq_enable();
}

/**
 * Próbuje przełączyć kontekst
 * Stan na wejściu: przerwania: obojętnie
 * Stan na wyjściu: przerwania: włączone, CIPL=0
 */
void
do_switch()
{
    int old=splsoftclock();
    KASSERT(old==0);

    /* Reorganizacja kolejek wątków */
    if ( end_ticks_reschedule <= clock_ticks )
        __resched();

    if(spinlock_trylock(&sprq)) //jeżeli odpalamy z wątku to powinno być ok
        __sched_yield();

    spl0();
    signal_handle(curthread);
}


/// Wymusza przełączanie kontekstu.
void
sched_yield()
{
    if (CIPL > 0) return;
    do_switch();
}

/// Dodaje wątek do kolejki programów działających.
void
sched_insert(thread_t *thr)
{
    spinlock_lock(&sprq);
    thr->thr_flags |= THREAD_RUN|THREAD_INRUNQ;
    list_insert_tail(&run_queue, thr);
    __resched();
    spinlock_unlock(&sprq);
}

static void _mutex_wakeup(mutex_t *m);

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


/// wersja mutex_unlock bez lockowania sprq
static void
__mutex_unlock(mutex_t *m)
{
    thread_t *n;

    spinlock_lock(&m->mtx_slock);
    _mutex_wakeup(m);
    m->mtx_owner = NULL;
    n = list_extract_first(&m->mtx_locking);
    if (n) {
        m->mtx_owner = n;
        _sched_wakeup(n);
    } else {
        m->mtx_locked = MUTEX_UNLOCKED;
    }
    spinlock_unlock(&m->mtx_slock);
}



/// Usypia działający wątek.
void
sched_unlock_and_wait(mutex_t *m)
{
    spinlock_lock(&sprq);
    curthread->thr_flags &= ~THREAD_RUN;
    curthread->thr_flags |= THREAD_SLEEP;
    __resched();
    __mutex_unlock(m);
    int old=splsoftclock();
    __sched_yield();
    splx(old);
}

/// Usypia działający wątek.
void
sched_wait(const char *fl, const char *fn, int l, const char *d)
{
    spinlock_lock(&sprq);
    curthread->thr_flags &= ~THREAD_RUN;
    curthread->thr_flags |= THREAD_SLEEP;
    THREAD_SET_WDESCR(curthread, fl, fn, l, d);
    __resched();
    int s = splsoftclock();
    __sched_yield();
    splx(s);
    curthread->thr_wdescr.descr = "running";
}

static inline void
_sched_wakeup(thread_t *n)
{
    if (!(n->thr_flags & THREAD_INRUNQ)) {
        n->thr_flags |= THREAD_INRUNQ;
        list_insert_tail(&run_queue, n);
        __resched();
    }

    n->thr_flags |= THREAD_RUN;
    n->thr_flags &= ~THREAD_SLEEP;
}

/**
 * Budzi wątek.
 * @param n Deskryptor wątku do obudzenia.
 */

void
sched_wakeup(thread_t *n)
{
    spinlock_lock(&sprq);
    _sched_wakeup(n);
    spinlock_unlock(&sprq);
}

/**
 * Usypia wątek na podaną ilość sekund
 * @param stime Czas w sekundach
 */

void
ssleep(uint stime, const char *fl, const char *fn, int l, const char *d)
{
    curthread->thr_wakeup_time = clock_ticks + stime * HZ;
    sched_wait(fl, fn, l, d);
}


void
imsleep(uint mtime, const char *fl, const char *fn, int l, const char *d)
{
    int ipl = CIPL;
    spl0();
    msleep(mtime, fl, fn, l, d);
    splx(ipl);
}

/**
 * Usypia wątek na podaną ilość milisekund
 * @param mtime Czas w milisekundach
 */

void
msleep(uint mtime, const char *fl, const char *fn, int l, const char *d)
{
    curthread->thr_wakeup_time = clock_ticks + (mtime * HZ)/1000;
    sched_wait(fl, fn, l, d);
}

/// Niszczy wątek.
void
sched_exit(thread_t *t)
{
    sched_exit_1(t);
    sched_exit_2(t);
}

void sched_exit_1(thread_t *t)
{
    spinlock_lock(&sprq);
    list_remove(&run_queue, t);
    __resched();
    t->thr_flags &= ~(THREAD_INRUNQ|THREAD_RUN);
}

void sched_exit_2(thread_t *t)
{
    if ( t == curthread ) {
        curthread = NULL;
        int old = splsoftclock();
        KASSERT(old <= IPL_SOFTCLOCK);
        __sched_yield();
        panic("sched_exit: shouldnt be here!");
    } else {
        spinlock_unlock(&sprq);
    }
}

void sched_dump(void);

void
sched_dump()
{
    spinlock_lock(&sprq);
    thread_t *c = curthread;
    do {
        kprintf("<%p>\n", c);
        c = list_next(&run_queue, c);
    } while (c != curthread);
    spinlock_unlock(&sprq);
}

/// Wybiera następny wątek do obsługi.
thread_t *
select_next_thread()
{
    //kprintf("%u %u %u\n", first_not_empty, list_length(&run_queue), list_length(&sched_queue[first_not_empty]));
    
    thread_t *p;
    
    if ( rescheduled == FALSE ) {
        p = curthread;
    } else {
        rescheduled = FALSE;
        p = NULL;
    }
    
    while ((p = list_next(&sched_queue[first_not_empty],p))) {
        if (p->thr_flags & THREAD_RUN) {
            return p;
        } else
        if( p->thr_flags & THREAD_SLEEP
                && p->thr_wakeup_time!=0
                && p->thr_wakeup_time <= clock_ticks ) {
            p->thr_flags |= THREAD_RUN;
            p->thr_flags &= ~THREAD_SLEEP;
            p->thr_wakeup_time = 0;
            return p;
        }
    }
    panic("Impossible to get here! runq=%u, curthr: %p, p: %p",
        list_length(&sched_queue[first_not_empty]), curthread, p);
    return 0;
}



/*============================================================================
 * Śpiące królewny... tzn kolejki.
 */

void
sleepq_init(sleepq_t *q)
{
    memzero(q, sizeof(*q));
    LIST_CREATE(&q->sq_waiting, thread_t, L_wait, FALSE);
}

void
sleepq_wait(sleepq_t *q, const char *fl, const char *fn, int l, const char *d)
{
    int s = splsoftclock();
    //int s = splhigh();
    list_insert_tail(&q->sq_waiting, curthread);
    curthread->thr_sleepq = q;
    UNSET(curthread->thr_flags, THREAD_INTRPT);
    SET(curthread->thr_flags, THREAD_SLEEPQ);
//    spinlock_lock(&sprq);
//    splx(s);
//    s = splsoftclock();
//    spinlock_unlock(&sprq);
//    UNSET(curthread->thr_flags,THREAD_RUN);
//    SET(curthread->thr_flags,THREAD_SLEEP|THREAD_SLEEPQ);
//    splx(s);
    sched_wait(fl, fn, l, d);
    splx(s);
}

void
sleepq_wakeup(sleepq_t *q)
{
    int s = splhigh();
    thread_t *t = NULL;
    while ( (t = list_extract_first(&q->sq_waiting)) ) {
        UNSET(t->thr_flags,THREAD_SLEEPQ);
        _sched_wakeup(t);
        t->thr_sleepq = 0;
    }
    splx(s);
}

void
sleepq_intrpt(thread_t *t)
{
    int s = splhigh();
    sleepq_t *q = t->thr_sleepq;
    if(!t->thr_sleepq) {
        splx(s);
        return;
    }
    UNSET(t->thr_flags, THREAD_SLEEPQ);
    SET(t->thr_flags, THREAD_INTRPT);
    _sched_wakeup(t);
    list_remove(&q->sq_waiting, t);
    t->thr_sleepq = 0;
    splx(s);
}

void
sleepq_destroy(sleepq_t *q)
{
    sleepq_wakeup(q);
}


/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/clock.h>
#include <sys/kprintf.h>

/// Kwant czasu przydzialny programom, w tykniêciach zegara.
int sched_quantum;

/// Kolejka programów dzia³aj±cych.
static list_t run_queue;
static int end_ticks;
/// Zamek zabezpieczaj±cy sekcje krytyczne planisty.
static spinlock_t sprq;

static inline thread_t * select_next_thread(void);
static void __sched_yield(void);
static inline void _sched_wakeup(thread_t *n);

/// Procedura inicjuj±ca program planisty.
void
sched_init()
{
    sched_quantum = 5;
    end_ticks = clock_ticks + sched_quantum;
    list_create(&run_queue, offsetof(thread_t, L_run_queue), TRUE);
    spinlock_init(&sprq);
    sched_insert(curthread);
}

/**
 * Podprogram planisty.
 *
 * Procedura jest uruchamiana przez program obs³ugi przerwania
 * zegara. Odlicza odpowiedni kwant czasu i zmienia kontekst.
 */

void
sched_action()
{
    if (clock_ticks >= end_ticks) {
        end_ticks = clock_ticks + sched_quantum;
        if (spinlock_trylock(&sprq)) {
            __sched_yield();
        }
    }
}

/**
 * Pomocnicza procedura zmieniaj±ca kontekst.
 *
 * Powinna byæ uruchamian tylko wewn±trz sekcji krytycznych
 * chronionych przez wiruj±cy zamek sprq. Jej zadanie to wybranie
 * kolejnego w±tku, wyj¶cie z sekcji krytycznej i zmiana kontekstu.
 */
void
__sched_yield()
{
    thread_t *n = select_next_thread();
//     kprintf("sched_yield.switch (cur=%p) (n=%p)\n", curthread, n);
    spinlock_unlock(&sprq);
    if (n == curthread) 
        return;
    thread_switch(n, curthread);
}


/// Wymusza prze³±czanie kontekstu.
void
sched_yield()
{
    spinlock_lock(&sprq);
    __sched_yield();
}

/// Dodaje w±tek do kolejki programów dzia³aj±cych.
void
sched_insert(thread_t *thr)
{
    spinlock_lock(&sprq);
    thr->thr_flags |= THREAD_RUN|THREAD_INRUNQ;
    list_insert_tail(&run_queue, thr);
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



/// Usypia dzia³aj±cy w±tek.
void
sched_unlock_and_wait(mutex_t *m)
{
    spinlock_lock(&sprq);
    curthread->thr_flags &= ~THREAD_RUN;
    curthread->thr_flags |= THREAD_SLEEP;
    __mutex_unlock(m);
    __sched_yield();
}

/// Usypia dzia³aj±cy w±tek.
void
sched_wait()
{
    spinlock_lock(&sprq);
//     kprintf("sched_wait(%p)\n", curthread);
    curthread->thr_flags &= ~THREAD_RUN;
    curthread->thr_flags |= THREAD_SLEEP;
    __sched_yield();
}

static inline void
_sched_wakeup(thread_t *n)
{
//     kprintf("sched_wakeup(%p) %p\n", curthread, n);
    if (!(n->thr_flags & THREAD_INRUNQ)) {
        curthread->thr_flags |= THREAD_INRUNQ;
        list_insert_tail(&run_queue, n);
    } 

    n->thr_flags |= THREAD_RUN;
    n->thr_flags &= ~THREAD_SLEEP;
}

/**
 * Budzi w±tek.
 * @param n Deskryptor w±tku do obudzenia.
 */

void
sched_wakeup(thread_t *n)
{
    spinlock_lock(&sprq);
    _sched_wakeup(n);
    spinlock_unlock(&sprq);
}

/**
 * Usypia w±tek na podan± ilo¶æ sekund
 * @param stime Czas w sekundach
 */

void
ssleep(uint stime)
{
    curthread->thr_wakeup_time = clock_ticks + stime * HZ;
    sched_wait();
}

/**
 * Usypia w±tek na podan± ilo¶æ milisekund
 * @param mtime Czas w milisekundach
 */

void
msleep(uint mtime)
{
    curthread->thr_wakeup_time = clock_ticks + (mtime * HZ)/1000;
    sched_wait();
}

/// Niszczy aktualny w±tek.
void
sched_exit()
{
    spinlock_lock(&sprq);
    list_remove(&run_queue, curthread);
    curthread->thr_flags &= ~(THREAD_INRUNQ|THREAD_RUN);
    __sched_yield();
}

/// Wybiera nastêpny w±tek do obs³ugi.
thread_t *
select_next_thread()
{
#define NEXTTHR() (thread_t*)list_next(&run_queue, p)
    thread_t *p = curthread;

    while ((p = NEXTTHR())) {
        if (p->thr_flags & THREAD_RUN)
            return p;
        else if(p->thr_flags & THREAD_SLEEP && p->thr_wakeup_time!=0 
                && p->thr_wakeup_time >= clock_ticks) {
            p->thr_flags |= THREAD_RUN;
            p->thr_flags &= ~THREAD_SLEEP;
            p->thr_wakeup_time = 0;
            return p;
        }
    }
    kprintf("!!!!!!!\n");
    while (TRUE); 
#undef NEXTTHR
}


/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
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
    if (n == curthread) return;
//     kprintf("sched_yield.switch (cur=%p) (n=%p)\n", curthread, n);
    spinlock_unlock(&sprq);
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

/**
 * Budzi w±tek.
 * @param n Deskryptor w±tku do obudzenia.
 */

void
sched_wakeup(thread_t *n)
{
    spinlock_lock(&sprq);
//     kprintf("sched_wakeup(%p) %p\n", curthread, n);
    if (!(n->thr_flags & THREAD_INRUNQ)) {
        curthread->thr_flags |= THREAD_INRUNQ;
        list_insert_tail(&run_queue, n);
    } 

    n->thr_flags |= THREAD_RUN;
    n->thr_flags &= ~THREAD_SLEEP;

    spinlock_unlock(&sprq);
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
    }
    while (TRUE); kprintf("!!!!!!!\n");
#undef NEXTTHR
}


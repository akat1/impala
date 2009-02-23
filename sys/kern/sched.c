#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/clock.h>
#include <sys/kprintf.h>

int sched_quantum;
list_t sched_runq;

static int end_ticks;
static spinlock_t sprq; // spinlock run queue

static inline thread_t * select_next_thread(void);

void
sched_init()
{
    sched_quantum = 5;
    end_ticks = clock_ticks + sched_quantum;
    list_create(&sched_runq, offsetof(thread_t, L_threads), TRUE);
    spinlock_init(&sprq);
    kprintf("round-robin scheduler with quantum: %u ticks\n", sched_quantum);

}

void
sched_action()
{
    if (clock_ticks >= end_ticks) {
        end_ticks = clock_ticks + sched_quantum;
        sched_yield();
    }
}

void
sched_yield()
{
    thread_t *n = select_next_thread();
    if (n == curthread) return;
    thread_switch(n, curthread);
}

thread_t *
select_next_thread()
{
    thread_t *c = curthread;
    do {
        c = c->runq_next;
    } while (c->thr_status != THREAD_RUN && !(c->thr_flags & THREAD_SYSCALL));
    return c;
}



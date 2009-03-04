#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/clock.h>
#include <sys/kprintf.h>

int sched_quantum;

static list_t run_queue;
static int end_ticks;
static spinlock_t sprq; // spinlock run queue

static inline thread_t * select_next_thread(void);

void
sched_init()
{
    sched_quantum = 5;
    end_ticks = clock_ticks + sched_quantum;
    list_create(&run_queue, offsetof(thread_t, L_run_queue), TRUE);
    spinlock_init(&sprq);
    kprintf("round-robin scheduler with quantum: %u ticks\n", sched_quantum);
    sched_insert(curthread);

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
    if (spinlock_trylock(&sprq)) {
        thread_t *n = select_next_thread();
        spinlock_unlock(&sprq);
        if (n == curthread) return;
        thread_switch(n, curthread);
    }
}


void
sched_insert(thread_t *thr)
{
    spinlock_lock(&sprq);
    list_insert_tail(&run_queue, thr);
    spinlock_unlock(&sprq);
}


thread_t *
select_next_thread()
{
    thread_t *p = (thread_t*)list_next(&run_queue, curthread);
    if (p == NULL) {
        kprintf("sched: cannot select next thread...\n");
        while (TRUE);
    }
    return p;
}



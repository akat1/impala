#include <sys/types.h>
#include <sys/thread.h>
#include <sys/libkutil.h>
#include <sys/sched.h>

thread_t *curthread;
thread_t *thread_idle;

list_t threads_list;

static thread_t thread_table[MAX_THREAD];
static thread_t *free_thr;
static thread_t *thread0;
static uint last_pid;

/*
 * Gdy sie przekreci last_pid, to nei sprawdzamy czy nie jedzie po pid'ach uzywanych
 * ... ale na razie to nie jest istotne.
 */

void
thread_init()
{
    int i;
    last_pid = 0;
    mem_zero(&thread_table, sizeof(thread_table));
    for (i = 0; i < MAX_THREAD-1; i++) {
        thread_table[i].thr_status = THREAD_SLEEP;
    }
    thread_table[MAX_THREAD-1].runq_next = 0;
    free_thr = &thread_table[0];
    curthread = thread0 = thread_create(0, 0, 0);
    thread0->thr_status = THREAD_RUN;

    list_create(&threads_list, offsetof(thread_t, L_threads), FALSE);
}


thread_t *
thread_create(int priv, addr_t entry, addr_t arg)
{
    if (free_thr) {
        thread_t *t = free_thr;
        t->thr_status = 0;
        t->thr_flags = 0;
        t->thr_tid = last_pid++;
        t->thr_priv = priv;
        t->thr_entry_point = entry;
        t->thr_entry_arg = arg;
        thread_context_init(&t->thr_context, priv, t->thr_stack);
        list_insert_tail(&threads_list, t);
        return t;
    } else {
        return 0;
    }
}

void
thread_run(thread_t *p)
{
    p->thr_flags |= THREAD_FRESH;
    p->thr_status = THREAD_RUN;
    sched_insert(p);

}

void
thread_exit(thread_t *t)
{
    t->thr_status = THREAD_DEAD;
}

void
thread_suspend(thread_t *t)
{
    t->thr_status = THREAD_SLEEP;
    if (t == curthread) sched_yield();
}


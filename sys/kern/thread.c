#include <sys/types.h>
#include <sys/thread.h>
#include <sys/libkutil.h>
#include <sys/sched.h>
#include <sys/kprintf.h>

thread_t *curthread;
thread_t *thread_idle;

list_t threads_list;

static thread_t thread_table[MAX_THREAD];
static uint last_pid;

static list_t __free_threads;

void
thread_init()
{
    last_pid = 0;
    mem_zero(&thread_table, sizeof(thread_table));
    list_create(&threads_list, offsetof(thread_t, L_threads), FALSE);
    list_create(&__free_threads, offsetof(thread_t, L_threads), FALSE);
    for (int i = 0; i < MAX_THREAD; i++) {
        list_insert_tail(&__free_threads, &thread_table[i]);
    }
    curthread = thread_create(0, 0, 0);
    curthread->thr_flags = THREAD_RUN;

}


thread_t *
thread_create(int priv, addr_t entry, addr_t arg)
{
    thread_t *free_thr = list_extract_first(&__free_threads);
    if (free_thr) {
        thread_t *t = free_thr;
        t->thr_flags = THREAD_NEW;
        t->thr_tid = last_pid++;
        t->thr_priv = priv;
        t->thr_entry_point = entry;
        t->thr_entry_arg = arg;
        thread_context_init(&t->thr_context, priv, t->thr_stack);
        list_insert_tail(&threads_list, t);
        return t;
    } else {
        kprintf("ERROR: no free threads!\n");
        return 0;
    }
}

void
thread_run(thread_t *p)
{
    sched_insert(p);
}

void
thread_exit(thread_t *t)
{
}

void
thread_suspend(thread_t *t)
{
    if (t == curthread) sched_yield();
}


#include <sys/types.h>
#include <sys/kprintf.h>
#include <sys/thread.h>
#include <sys/kthread.h>
#include <sys/clock.h>
#include <sys/sched.h>
#include <sys/syscall.h>
#include <sys/libkutil.h>

void kmain(void);
void test(void);

void
kmain()
{
    kprintf("==> ImpalaOS\n");
    kprintf("** expect many stupid messages!\n");
    thread_init();
    sched_init();
    clock_init();

    test();

    for (;;) {
    }
}

void k(void *arg);

spinlock_t sp;

void
k(void *arg)
{
    int id = (int)arg;
    kprintf("kthr %u started\n", id);
    while (TRUE) {
        spinlock_lock(&sp);
        kprintf("%u: enter\n", id);
        for (int i = 0; i < 30000000; i++);
        kprintf("%u: leave\n", id);
        spinlock_unlock(&sp);
        for (int i = 0; i < 30000000; i++);

    }
}

void
test()
{
    static kthread_t t0, t1, t2, t3;
    spinlock_init(&sp);
    kprintf("creating...\n");
    kthread_create(&t0, k, (void*) 0);
    kthread_create(&t1, k, (void*) 1);
    kthread_create(&t2, k, (void*) 2);
    kthread_create(&t3, k, (void*) 3);
    kprintf("created\n");
}

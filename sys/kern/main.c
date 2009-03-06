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

void ki(void *arg);
void ko(void *arg);
static cqueue_t kolejka;

#define ASLEEP() for (int _x = 0; _x < 4999999; _x++)

struct dane {
    int no;
    list_node_t L_ioq;
};

struct dane DANE[1000];
#define IDX(i) (i%1000)

void
ki(void *arg)
{
    for (int i = 0; i < 100; i++) {  
        ASLEEP();
        kprintf("inq %u\n",i);
        DANE[IDX(i)].no = i;
        cqueue_insert(&kolejka, &DANE[IDX(i)]);
    }
    while (TRUE);
}

void
ko(void *arg)
{
    int id = (int)arg;
    int i = 0;
    struct dane *d;
    kprintf("%u: work\n", id);
    while (i < 40 && (d = cqueue_extract(&kolejka))) {
        ASLEEP();
        kprintf("%u: %u\n", id, d->no);
        i++;
    }
    while (TRUE);
}


void
test()
{
    static kthread_t t0 , t1, t2, t3;
    cqueue_init(&kolejka, offsetof(struct dane, L_ioq));
    kprintf("creating...\n");
    kthread_create(&t0, ki, (void*) 0);
    kthread_create(&t1, ko, (void*) 1);
    kthread_create(&t2, ko, (void*) 2);
    kthread_create(&t3, ko, (void*) 3);
    kprintf("created\n");
}

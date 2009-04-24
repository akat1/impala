/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/kprintf.h>
#include <sys/thread.h>
#include <sys/kthread.h>
#include <sys/clock.h>
#include <sys/sched.h>
#include <sys/syscall.h>
#include <sys/libkutil.h>
#include <sys/device.h>
#include <sys/fcntl.h>
#include <machine/cpu.h>
#include <sys/uio.h>
#include <sys/vm.h>
#include <sys/utils.h>
#include <sys/errno.h>
#include <sys/kmem.h>

void kmain(void);
static void print_welcome(void);
static void init_kernel(void);
/// G��wna procedura j�dra.
static void dev_test(void);


void
kmain()
{
    print_welcome();
    init_kernel();
    DEBUGF("running tests...");
    dev_test();
    for (;;);
}

semaph_t sem;
void tf0(void *a);
void tf1(void *b);

void 
tf0(void *a)
{
    semaph_wait(&sem);
    kprintf("elo\n");
    for (;;);
}

void
tf1(void *b)
{
    semaph_wait(&sem);
    kprintf("elo2\n");
    for (;;);
}

void
dev_test()
{
    static kthread_t t0, t1;
    semaph_init(&sem);
    kthread_create(&t0, tf0, NULL);
    kthread_create(&t1, tf1, NULL);
    for (int i = 0; i < 0xfffffff; i++ );
    kprintf("POST\n");
    semaph_post(&sem);
    for (int i = 0; i < 0xfffffff; i++ );
    kprintf("POST2\n");
    semaph_post(&sem);

}

void
print_welcome()
{
    extern int kernel_start, kernel_end;
    kprintf("Copyright (C) 2009\n");
    kprintf("   Mateusz Kocielski, Artur Koninski, Pawel Wieczorek\n");
    kprintf("   http://trzask.codepainters.com/impala/trac/\n");
    kprintf("Copyright (C) 2009\n");
    kprintf("   Department of Computer Science. "
            "University of Wroclaw. Poland.\n");
    kprintf("   http://www.ii.uni.wroc.pl/\n");
    kprintf("----\n");
    kprintf("Impala Operating System, build at %s %s\n", __TIME__, __DATE__);
    kprintf("kernel loaded at: %p+%u kB\n", &kernel_start, 
        ((uintptr_t)&kernel_end - (uintptr_t)&kernel_start)/1024);
    
}

void
init_kernel()
{
    vm_init();
    kmem_init();
    thread_init();
    sched_init();
    clock_init();
    dev_init();
    kprintf("kernel initialized\n");
}

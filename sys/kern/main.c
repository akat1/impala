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
#include <machine/cpu.h>
#include <sys/vm.h>
#include <sys/utils.h>
#include <sys/errno.h>
#include <sys/kmem.h>

void kmain(void);
static void print_welcome(void);
static void init_kernel(void);
/// G³ówna procedura j±dra.

static void vm_test(void);
static void kmem_test(void);

void
kmain()
{
    print_welcome();
    init_kernel();
    DEBUGF("running tests...");
    vm_test();
    kmem_test();
    for (;;);
}

void
vm_test()
{
    vm_segment_t *kdata = &vm_kspace.seg_data;
    TRACE_IN("start");
    vm_addr_t a[10];
    a[0] = vm_segment_alloc(kdata, PAGE_SIZE*5);
    a[1] = vm_segment_alloc(kdata, PAGE_SIZE*4);
    kprintf("a0=%p a1=%p\n", a[0], a[1]);
    vm_segment_free(kdata, a[0]+PAGE_SIZE, PAGE_SIZE*4);
    a[2] = vm_segment_alloc(kdata, PAGE_SIZE);
    kprintf("a2=%p\n", a[2]);
    
    TRACE_IN("stop");
}


void
kmem_test()
{
    void *a[10];
    a[0] = kmem_alloc(13, KM_SLEEP);
    kprintf("a0=%p\n", a[0]);
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
    kprintf("kernel initialized\n");
}

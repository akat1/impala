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

void kmain(void);


/// G³ówna procedura j±dra.
void
kmain()
{
    kprintf("==> ImpalaOS\n");
    kprintf("** expect many stupid messages!\n");
    thread_init();
    sched_init();
    clock_init();

    for (;;) {
    }
}


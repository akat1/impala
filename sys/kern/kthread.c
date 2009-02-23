#include <sys/types.h>
#include <sys/thread.h>
#include <sys/kthread.h>
#include <sys/sched.h>
#include <sys/kprintf.h>

static void __kthr(kthread_t *arg);

void
kthread_create(kthread_t *kthr, kthread_entry_f *f, void *arg)
{
//     TRACE_IN("kthr=%p entry=%p arg=%p", kthr, f, arg);
    kthr->kt_arg = arg;
    kthr->kt_entry = f;
    kthr->kt_thread = thread_create(0, (void*)__kthr, kthr);
    thread_run(kthr->kt_thread);
}


void
__kthr(kthread_t *arg)
{
//     TRACE_IN("arg=%p enter",  arg);
    arg->kt_entry(arg->kt_arg);
    thread_exit(arg->kt_thread);
//     TRACE_IN("arg=%p exit");
    sched_yield();

}


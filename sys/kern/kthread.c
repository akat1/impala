#include <sys/types.h>
#include <sys/thread.h>
#include <sys/kthread.h>
#include <sys/sched.h>
#include <sys/kprintf.h>

static void __kthr(kthread_t *arg);

void
kthread_create(kthread_t *kthr, kthread_entry_f *f, void *arg)
{
    kthr->kt_arg = arg;
    kthr->kt_entry = f;
    kthr->kt_thread = thread_create(0, (void*)__kthr, kthr);
    thread_run(kthr->kt_thread);
}


void
__kthr(kthread_t *arg)
{
    arg->kt_entry(arg->kt_arg);
    thread_exit(arg->kt_thread);
    sched_exit();

}


/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/kthread.h>
#include <sys/sched.h>
#include <sys/kprintf.h>

/// Wej�ciowa procedura dla obs�ugi w�tku.
static void __kthr(kthread_t *arg);

/**
 * Tworzy nowy w�tek po stronie j�dra.
 * @param thr referencja do deskryptora w�tku.
 * @param f adres procedury wej�ciowej.
 * @param arg adres przekazany jako argument do procedury wej�ciowej.
 * 
 */
void
kthread_create(kthread_t *kthr, kthread_entry_f *f, void *arg)
{
    TRACE_IN("kthr=%p f=%p arg=%p", kthr, f, arg);
    kthr->kt_arg = arg;
    kthr->kt_entry = f;
    kthr->kt_thread = thread_create(0, (void*)__kthr, kthr);
    sched_insert(kthr->kt_thread);
}

void
__kthr(kthread_t *arg)
{
//    TRACE_IN("elo");
//    TRACE_IN("arg=%p entry=%p entry_arg=%p", arg, arg->kt_entry, arg->kt_arg);
    arg->kt_entry(arg->kt_arg);
    sched_exit();
}

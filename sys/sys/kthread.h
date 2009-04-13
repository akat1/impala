/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_KTHREAD
#define __SYS_KTHREAD


typedef void kthread_entry_f(void *);

/// W±tek j±dra.
struct kthread {
    /// w±tek procesora.
    thread_t *kt_thread;
    /// argument procedury wej¶ciowej
    void     *kt_arg;
    /// adres procedury wej¶ciowej
    kthread_entry_f *kt_entry;;
};

#ifdef __KERNEL
void kthread_create(kthread_t *thr, kthread_entry_f *p, void *arg);
void kthread_cancel(kthread_t *thr);
#endif

#endif


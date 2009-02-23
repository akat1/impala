#ifndef __SYS_KTHREAD
#define __SYS_KTHREAD


typedef void kthread_entry_f(void *);

struct kthread {
    thread_t *kt_thread;
    void     *kt_arg;
    kthread_entry_f *kt_entry;;
};

#ifdef __KERNEL
void kthread_create(kthread_t *thr, kthread_entry_f *p, void *arg);
void kthread_cancel(kthread_t *thr);
#endif

#endif


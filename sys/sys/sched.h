#ifndef __SYS_SCHED_H
#define __SYS_SCHED_H

#ifdef __KERNEL
extern int sched_quantum;

void sched_init(void);
void sched_action(void);
void sched_yield(void);

void sched_insert(thread_t *thr);
void sched_remove(thread_t *thr);

void sched_wait(void);
void sched_wakeup(thread_t *n);

#endif



#endif


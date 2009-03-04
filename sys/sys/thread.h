#ifndef __SYS_THREAD_H
#define __SYS_THREAD_H

#include <machine/thread.h>
#include <machine/atomic.h>
#include <sys/list.h>
enum {
    MAX_THREAD = 0x10,
    THREAD_STACK_SIZE = 0x4000
};


struct spinlock {
    uint32_t    _dlock;
};

struct thread {
    thread_context  thr_context;
    int             thr_tid;
    addr_t          thr_entry_point;
    addr_t          thr_entry_arg;
    int             thr_priv;
    int             thr_flags;
    char            thr_stack[THREAD_STACK_SIZE];
    list_node_t     L_run_queue;
    list_node_t     L_threads;
};




enum THREAD_FLAGS {
    THREAD_NEW       = 1 << 0, // in-creating
    THREAD_ZOMBIE    = 1 << 1, // in-destroying
    THREAD_RUN       = 1 << 2, // are in run_queue
    THREAD_SYSCALL   = 1 << 3, // are in syscall handler
    THREAD_SLEEP     = 1 << 4, // sleeped
};

enum {
    SPINLOCK_UNLOCK,
    SPINLOCK_LOCK
};

#ifdef __KERNEL
extern thread_t *curthread;     // nie lepiej curthread ?
extern thread_t *thread_idle;
extern list_t threads_list;

void thread_init(void);
thread_t *thread_create(int priv, addr_t entry, addr_t arg);
void thread_run(thread_t *p);
void thread_exit(thread_t *p);
void thread_suspend(thread_t *t);

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
bool mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);

void condvar_init(condvar_t *cv, mutex_t *m);
void condvar_wait(condvar_t *cv);
void condvar_notify_one(condvar_t *cv);
void condvar_notify_all(condvar_t *cv);


static inline void
spinlock_init(spinlock_t *sp)
{
    sp->_dlock = SPINLOCK_UNLOCK;
}

static inline void
spinlock_lock(spinlock_t *sp)
{
    while (atomic_change32(&sp->_dlock, SPINLOCK_LOCK) == SPINLOCK_LOCK);
}

static inline void
spinlock_unlock(spinlock_t *sp)
{
//      atomic_change32(&sp->_dlock, SPINLOCK_UNLOCK);
      sp->_dlock = SPINLOCK_UNLOCK;
}

static inline bool
spinlock_trylock(spinlock_t *sp)
{
    return atomic_change32(&sp->_dlock, SPINLOCK_LOCK) == SPINLOCK_UNLOCK;
}



#endif
#endif


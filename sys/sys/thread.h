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
    int             thr_status;
    int             thr_priv;
    int             thr_flags;
    char            thr_stack[THREAD_STACK_SIZE];
    thread_t       *runq_next;
    list_node_t     L_run_queue;
    list_node_t     L_threads;
};


enum THREAD_STATUS {
    THREAD_RUN,
    THREAD_SLEEP,
    THREAD_DEAD
};


enum THREAD_FLAGS {
    THREAD_FRESH    = 1 << 0,
    THREAD_SYSCALL  = 1 << 1
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
    atomic_change32(&sp->_dlock, SPINLOCK_UNLOCK);
}



#endif
#endif


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
    volatile int    _dlock;
};


struct thread {
    thread_context  thr_context;
    int             thr_tid;
    addr_t          thr_entry_point;
    addr_t          thr_entry_arg;
    int             thr_priv;
    int             thr_flags;
    uint            thr_wakeup_time;
    char            thr_stack[THREAD_STACK_SIZE];
    proc_t         *thr_proc;
    list_node_t     L_run_queue;
    list_node_t     L_threads;
    list_node_t     L_wait;
};

struct mutex {
    thread_t     *mtx_owner;
    int           mtx_locked;
    int           mtx_flags;
    spinlock_t    mtx_slock;
    list_t        mtx_locking; // thread_t.L_wait
    list_t        mtx_waiting; // thread_t.L_wait
};

struct cqueue {
    mutex_t     q_mtx;
    list_t      q_data;
};


enum THREAD_FLAGS {
    THREAD_NEW       = 1 << 0, // in-creating
    THREAD_ZOMBIE    = 1 << 1, // in-destroying
    THREAD_RUN       = 1 << 2, // are running
    THREAD_SYSCALL   = 1 << 3, // are in syscall handler
    THREAD_SLEEP     = 1 << 4, // sleeped
    THREAD_INPROC    = 1 << 5, // connected to user process
    THREAD_INRUNQ    = 1 << 6  // are in run-queue
};

enum {
    MUTEX_NORMAL     = 0,
    MUTEX_CONDVAR    = 1 << 0,
    MUTEX_WAKEUP_ONE = 1 << 1,
    MUTEX_WAKEUP_ALL = 1 << 2
};


enum {
    MUTEX_LOCKED,
    MUTEX_UNLOCKED
};



enum {
    SPINLOCK_UNLOCK,
    SPINLOCK_LOCK
};

#ifdef __KERNEL
extern thread_t *curthread;     // nie lepiej curthread ?
extern thread_t *thread_idle;
extern list_t threads_list;     // thread_t.L_threads

void thread_init(void);
thread_t *thread_create(int priv, addr_t entry, addr_t arg);

// do wywalenia
void thread_suspend(thread_t *t);

void mutex_init(mutex_t *m, int flags);
void mutex_lock(mutex_t *m);
bool mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);
void mutex_destroy(mutex_t *m);
void mutex_wait(mutex_t *m);
void mutex_wakeup(mutex_t *m);
void mutex_wakeup_all(mutex_t *m);

void cqueue_init(cqueue_t *m, int off);
void cqueue_insert(cqueue_t *m, void *d);
void *cqueue_extract(cqueue_t *m);
void cqueue_shutdown(cqueue_t *m);

static inline void
spinlock_init(spinlock_t *sp)
{
    sp->_dlock = SPINLOCK_UNLOCK;
}

static inline void
spinlock_lock(spinlock_t *sp)
{
    while (atomic_change_int(&sp->_dlock, SPINLOCK_LOCK) == SPINLOCK_LOCK);
}

static inline void
spinlock_unlock(spinlock_t *sp)
{
      sp->_dlock = SPINLOCK_UNLOCK;
}

static inline bool
spinlock_trylock(spinlock_t *sp)
{
    return atomic_change_int(&sp->_dlock, SPINLOCK_LOCK) == SPINLOCK_UNLOCK;
}

#endif
#endif


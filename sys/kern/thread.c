#include <sys/types.h>
#include <sys/thread.h>
#include <sys/libkutil.h>
#include <sys/sched.h>
#include <sys/kprintf.h>

thread_t *curthread;
thread_t *thread_idle;

list_t threads_list;

static thread_t thread_table[MAX_THREAD];
static uint last_pid;

static list_t __free_threads;

/*=============================================================================
 * Obsluga watkow.
 */

void
thread_init()
{
    last_pid = 0;
    mem_zero(&thread_table, sizeof(thread_table));
    list_create(&threads_list, offsetof(thread_t, L_threads), FALSE);
    list_create(&__free_threads, offsetof(thread_t, L_threads), FALSE);
    for (int i = 0; i < MAX_THREAD; i++) {
        list_insert_tail(&__free_threads, &thread_table[i]);
    }
    curthread = thread_create(0, 0, 0);
    curthread->thr_flags = THREAD_RUN;

}


thread_t *
thread_create(int priv, addr_t entry, addr_t arg)
{
    thread_t *free_thr = list_extract_first(&__free_threads);
    if (free_thr) {
        thread_t *t = free_thr;
        t->thr_flags = THREAD_NEW;
        t->thr_tid = last_pid++;
        t->thr_priv = priv;
        t->thr_entry_point = entry;
        t->thr_entry_arg = arg;
        thread_context_init(&t->thr_context, priv, t->thr_stack);
        list_insert_tail(&threads_list, t);
        return t;
    } else {
        kprintf("ERROR: no free threads!\n");
        return 0;
    }
}

void
thread_run(thread_t *p)
{
    sched_insert(p);
}

void
thread_exit(thread_t *t)
{
}

void
thread_suspend(thread_t *t)
{
    if (t == curthread) sched_yield();
}


/*=============================================================================
 * Obsluga mutexow. (FIFO).
 */


static void _mutex_wakeup(mutex_t *m);


void
mutex_init(mutex_t *m, int flags)
{
    mem_zero(m, sizeof(mutex_t));
    m->mtx_flags = flags;
    m->mtx_locked = MUTEX_UNLOCKED;
    spinlock_init(&m->mtx_slock);
    list_create(&m->mtx_locking, offsetof(thread_t, L_wait), FALSE);
    if (flags & MUTEX_CONDVAR) {
        list_create(&m->mtx_waiting, offsetof(thread_t, L_wait), FALSE);
    }
}

void
mutex_destroy(mutex_t *m)
{
}


void
mutex_lock(mutex_t *m)
{
    if ( atomic_change_int(&m->mtx_locked, MUTEX_LOCKED) == MUTEX_UNLOCKED) {
        m->mtx_owner = curthread;
//         kprintf("mutex_lock:%p\n", m->mtx_owner);
    } else {
        // KASSERT(m->mtx_owner != curthread);
        spinlock_lock(&m->mtx_slock);
        list_insert_head(&m->mtx_locking, curthread);
        spinlock_unlock(&m->mtx_slock);
        sched_wait();

    }
}

void
mutex_unlock(mutex_t *m)
{
    thread_t *n;

    // KASSERT(m->mtx_owner == curthread)
    spinlock_lock(&m->mtx_slock);
//     kprintf("mutex_unlock:%p\n", m->mtx_owner);
    _mutex_wakeup(m);
    m->mtx_owner = NULL;
    n = list_extract_first(&m->mtx_locking);
    if (n) {
        m->mtx_owner = n;
//         kprintf("trying to wakeup cur=%p n=%p\n", curthread, n);
        sched_wakeup(n);
    } else {
        m->mtx_locked = MUTEX_UNLOCKED;
    }
    spinlock_unlock(&m->mtx_slock);
}

void
_mutex_wakeup(mutex_t *m)
{
    int l = 0;
    if (m->mtx_flags & MUTEX_WAKEUP_ALL) {
        l = list_length(&m->mtx_waiting);
    } else
    if (m->mtx_flags & MUTEX_WAKEUP_ONE) {
        l = (list_length(&m->mtx_waiting)==0)? 0 : 1;
    }
    if (l)
//     kprintf("trying to wakeup %u\n", l);
    for (int i = 0; i < l; i++) {
        thread_t *n = list_extract_first(&m->mtx_waiting);
//         kprintf("(%p:%p) waiting + %p\n", curthread, m->mtx_owner, n);
//         list_insert_tail(&m->mtx_locking, n);
        sched_wakeup(n);
    }
    m->mtx_flags &= ~(MUTEX_WAKEUP_ONE|MUTEX_WAKEUP_ALL);
}

bool
mutex_trylock(mutex_t *m)
{
    if (atomic_change_int(&m->mtx_locked, MUTEX_LOCKED) == MUTEX_UNLOCKED) {
        m->mtx_owner = curthread;
        return TRUE;
    } else {
        return FALSE;
    }
}

void
mutex_wait(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
//     kprintf("mutex_wait:%p\n", m->mtx_owner);
    list_insert_tail(&m->mtx_waiting, curthread);
    spinlock_unlock(&m->mtx_slock);
    mutex_unlock(m);
//     kprintf("@mutex_wait cur=%p owner=%p going sleep\n",curthread, m->mtx_owner);
    sched_wait();
//     kprintf("@mutex_wait:%p waked up\n", m->mtx_owner);
//     mutex_lock(m);
}

void
mutex_wakeup(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
//     kprintf("mutex_wakeup:%p\n", m->mtx_owner);
    m->mtx_flags |= MUTEX_WAKEUP_ONE;
    spinlock_unlock(&m->mtx_slock);
}

void
mutex_wakeup_all(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
    m->mtx_flags |= MUTEX_WAKEUP_ALL;
    spinlock_unlock(&m->mtx_slock);
}

/*=============================================================================
 * Obsluga kolejek
 */

void
cqueue_init(cqueue_t *q, int off)
{
    mutex_init(&q->q_mtx, MUTEX_CONDVAR);
    list_create(&q->q_data, off, FALSE);
}

void
cqueue_shutdown(cqueue_t *q)
{
}

void
cqueue_insert(cqueue_t *q, void *d)
{
    mutex_lock(&q->q_mtx);
    list_insert_tail(&q->q_data, d);
//     kprintf("cqI: wakeup\n");
    mutex_wakeup(&q->q_mtx);
    mutex_unlock(&q->q_mtx);
}

void*
cqueue_extract(cqueue_t *q)
{
    void *p;
    mutex_lock(&q->q_mtx);
    while ( (p = list_extract_first(&q->q_data)) == NULL ) {
//         kprintf("cqE: wait\n");
        mutex_wait(&q->q_mtx);
    }
    mutex_unlock(&q->q_mtx);
    return p;
}


#include <sys/types.h>
#include <sys/thread.h>
#include <stdlib.h>
#include <pthread.h>
#include "pthread_priv.h"

bool _pthread_initialized = FALSE;
volatile pthread_t _current_pthread;
pthread_spinlock_t _pthread_slock;

static pthread_t thread0;

void
_pthread_rt()
{
    PTHREAD_LOG("initializing POSIX thread runtime (debug build)\n");
    _pthread_initialized = TRUE;
    thread0 = malloc( sizeof(struct pthread) );
    thread0->pth_id = thr_getid();
    pthread_spin_init(&_pthread_slock, PTHREAD_PROCESS_PRIVATE);
}

pthread_t
_pthread_self()
{
    pthread_t cp = thr_getarg();
    if (cp == 0) {
        return thread0;
    }
    return cp;
}

pthread_t
pthread_self()
{
    pthread_t r;
    _PTHREAD_LOCK();
    r = _pthread_self();
    _PTHREAD_UNLOCK();
    return r;
}

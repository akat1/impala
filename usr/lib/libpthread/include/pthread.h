/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#ifndef __PTHREAD_H
#define __PTHREAD_H

#include <sys/types.h>
#include <sys/list.h>
#include <machine/atomic.h>

enum {
    PTHREAD_SUNLOCK,
    PTHREAD_SLOCK
};

enum {
    PTHREAD_PROCESS_SHARED,
    PTHREAD_PROCESS_PRIVATE
};

typedef struct pthread *pthread_t;
typedef struct pthread_attr pthread_attr_t;
typedef struct pthread_mutexattr pthread_mutexattr_t;
typedef struct pthread_condattr pthread_condattr_t;
typedef struct pthread_mutex pthread_mutex_t;
typedef struct pthread_cond pthread_cond_t;
typedef struct pthread_spinlock pthread_spinlock_t;

typedef void *(*pthread_entry)(void *);

struct pthread_attr {
    size_t      stacksize;
    void        *stackaddr;
};

struct pthread {
    tid_t           pth_id;
    pthread_entry   pth_entry;
    void           *pth_entry_arg;
    pthread_attr_t  pth_attr;
    void           *pth_exit;
    void           *pth_retval;
    list_node_t     L_pthreads;
};

struct pthread_mutexattr {
    int dummy;
};

struct pthread_condattr {
    int dummy;
};

struct pthread_mutex {
    mid_t               pm_id;
    pthread_t           pm_owner;
};

struct pthread_cond {
    pthread_mutex_t     *pc_mtx;
};

struct pthread_spinlock {
    volatile int    _dlock;
};


int pthread_attr_init(pthread_attr_t *);
int pthread_attr_destroy(pthread_attr_t *);
int pthread_attr_setstacksize(pthread_attr_t *, size_t);
int pthread_attr_getstacksize(const pthread_attr_t *, size_t *);
int pthread_attr_setstackaddr(pthread_attr_t *, void *);
int pthread_attr_getstackaddr(const pthread_attr_t *, void **);

int pthread_mutexattr_init(pthread_mutexattr_t *);
int pthread_mutexattr_destroy(pthread_mutexattr_t *);

int pthread_condattr_init(pthread_mutexattr_t *);
int pthread_condattr_destroy(pthread_mutexattr_t *);

int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int pthread_mutex_trylock(pthread_mutex_t *);
int pthread_mutex_destroy(pthread_mutex_t *);

int pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
int pthread_cond_signal(pthread_cond_t *);
int pthread_cond_broadcast(pthread_cond_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int pthread_mutex_trylock(pthread_mutex_t *);

int pthread_create(pthread_t *, const pthread_attr_t *, pthread_entry, void *);
int pthread_detach(pthread_t);
int pthread_cancel(pthread_t);
int pthread_join(pthread_t, void **);
pthread_t pthread_self(void);
void pthread_exit(void *);
int pthread_yield(void);

static inline void
pthread_spin_init(pthread_spinlock_t *sp, int pshared)
{
    sp->_dlock = PTHREAD_SUNLOCK;
}

static inline void
pthread_spin_lock(pthread_spinlock_t *sp)
{
    while (atomic_change_int(&sp->_dlock, PTHREAD_SLOCK) == PTHREAD_SLOCK) {
        pthread_yield();
    }
}

static inline void
pthread_spin_unlock(pthread_spinlock_t *sp)
{
      sp->_dlock = PTHREAD_SUNLOCK;
}

static inline bool
pthread_spin_trylock(pthread_spinlock_t *sp)
{
    return atomic_change_int(&sp->_dlock, PTHREAD_SLOCK) == PTHREAD_SUNLOCK;
}

static inline void
pthread_spin_destroy(pthread_spinlock_t *sp)
{
}

#endif


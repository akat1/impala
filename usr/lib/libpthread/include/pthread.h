/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
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

typedef struct pthread *pthread_t;
typedef struct pthread_attr pthread_attr_t;
typedef struct pthread_mutexattr pthread_mutexattr_t;
typedef struct pthread_condattr pthread_condattr_t;
typedef struct pthread_mutex pthread_mutex_t;
typedef struct pthread_cond pthread_cond_t;

typedef void *(*pthread_entry)(void *);

struct pthread {
    tid_t           pth_tid;
    pthread_entry   pth_entry;
    void            *pth_entry_arg;
};

struct pthread_mutex {
    int                 pmtx_id;
    tid_t               pmtx_owner;
};

struct pthread_cond {
    pthread_mutex_t     *pcn_mtx;
};

int pthread_create(pthread_t *, const pthread_attr_t *, pthread_entry, void *);
int pthread_cancel(pthread_t);
int pthread_join(pthread_t, void **);
pthread_t pthread_self(void);
void pthread_exit(void *);
int pthread_yield(void);

#endif


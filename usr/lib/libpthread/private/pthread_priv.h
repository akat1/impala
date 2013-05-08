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
 * $Id: pthread.h 486 2009-06-25 07:51:47Z wieczyk $
 */

#ifndef __PTHREAD_PRIV_H
#define __PTHREAD_PRIV_H
#define PTHREAD_DEBUG

extern bool _pthread_initialized;
extern pthread_spinlock_t _pthread_slock;
extern list_t _pthread_list;

void _pthread_rt(void);
pthread_t _pthread_self(void);

#define __PTHREAD_INITIALIZE()\
    do {\
        if (!_pthread_initialized) {\
            _pthread_rt();\
        }\
    } while(0);

#define _PTHREAD_LOCK() pthread_spin_lock(&_pthread_slock)
#define _PTHREAD_UNLOCK() pthread_spin_unlock(&_pthread_slock)

#ifdef PTHREAD_DEBUG
#include <stdio.h>
#   define PTHREAD_LOG(fmt, a...) if (getenv("PTHREAD_DEBUG") != 0) \
        fprintf(stderr, "\033[33mPTHREAD: " fmt "\033[0m\n", ## a)
#else
#   define PTHREAD_LOG(fmt, a...)
#endif


#endif


/*
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

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/utils.h>
#include <sys/syscall.h>
#include <sys/signal.h>
#include <sys/proc.h>
#include <sys/string.h>

typedef struct sigprocmask_args sigprocmask_args_t;

struct sigprocmask_args {
    int how;
    sigset_t *set;
    sigset_t *oldset;
};

errno_t sc_sigprocmask(thread_t *t, syscall_result_t *r, sigprocmask_args_t *args);

errno_t
sc_sigprocmask(thread_t *t, syscall_result_t *r, sigprocmask_args_t *args)
{
    sigset_t sigblock;

    *(args->set) &= ~(SIGKILL|SIGSTOP);

    switch(args->how) {
        case SIG_BLOCK:
            if ( args->oldset != NULL ) {
                copyout(args->oldset, &t->thr_sigblock, sizeof(sigset_t));
            }
            copyin(&sigblock, args->set, sizeof(sigset_t));
            sigblock |= t->thr_sigblock;
            memcpy(&t->thr_sigblock, &sigblock, sizeof(sigset_t));
            r->result = 0;
            return EOK;
        case SIG_UNBLOCK:
            if ( args->oldset != NULL ) {
                copyout(args->oldset, &t->thr_sigblock, sizeof(sigset_t));
            }
            copyin(&sigblock, args->set, sizeof(sigset_t));
            sigblock &= ~t->thr_sigblock;
            memcpy(&t->thr_sigblock, &sigblock, sizeof(sigset_t));
            r->result = 0;
            return EOK;
        case SIG_SETMASK:
            if ( args->oldset != NULL ) {
                copyout(args->oldset, &t->thr_sigblock, sizeof(sigset_t));
            }
            if ( args->set != NULL ) {
                copyin(&t->thr_sigblock, args->set, sizeof(sigset_t));
            }
            r->result = 0;
            return EOK;
        default:
            return -EINVAL;
    }

    /* NOT REACHED */
}


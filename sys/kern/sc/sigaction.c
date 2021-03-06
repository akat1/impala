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

typedef struct sigaction_args sigaction_args_t;

struct sigaction_args {
    int sig;
    sigaction_t *act;
    sigaction_t *oldact;
};

errno_t sc_sigaction(thread_t *p, syscall_result_t *r, sigaction_args_t *args);

errno_t
sc_sigaction(thread_t *t, syscall_result_t *r, sigaction_args_t *args)
{
    proc_t *p = t->thr_proc;

    if ( args->sig < 1 || args->sig > _NSIG ||
         args->sig == SIGSTOP || args->sig == SIGKILL ) {

        r->result = -1;
        return EINVAL;
    }

    if ( args->oldact ) {
        copyout(args->oldact, &p->p_sigact[args->sig], sizeof(sigaction_t));
    }

    if ( args->act ) {
        copyin(&p->p_sigact[args->sig], args->act, sizeof(sigaction_t));
    }

    r->result = 0;
    return EOK;
}

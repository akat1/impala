/*
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

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/utils.h>
#include <sys/syscall.h>

typedef struct sc_exit_args sc_exit_args;

struct sc_exit_args {
    int error;
};

errno_t sc_exit(thread_t *p, syscall_result_t *r, sc_exit_args *args);

errno_t
sc_exit(thread_t *t, syscall_result_t *r, sc_exit_args *args)
{
    proc_t *p = t->thr_proc;
    thread_t *t_iter;

    // ustawiamy status procesu 
    p->p_status = args->error;
    r->result = args->error;

    t_iter = (thread_t *)list_head(&p->p_threads);

    #define NEXTTHR() (thread_t *)list_next(&p->p_threads, t_iter)
    {
        t_iter->thr_flags = THREAD_ZOMBIE;
        if ( t_iter != curthread )
            sched_exit(t_iter);
    } while ((t_iter = NEXTTHR()))
    #undef NEXTTHR

    // proces zombie 
    p->p_flags = PROC_ZOMBIE;
    
    sched_exit(curthread);

    return EOK;
}


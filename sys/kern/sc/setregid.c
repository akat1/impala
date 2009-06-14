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
 * $Id: setregid.c 249 2009-05-24 15:14:24Z shm $
 */

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/utils.h>
#include <sys/syscall.h>

typedef struct setregid_args setregid_args;

struct setregid_args {
    gid_t rgid;
    gid_t egid;
};

errno_t sc_setregid(thread_t *t, syscall_result_t *r, setregid_args *args);

errno_t
sc_setregid(thread_t *t, syscall_result_t *r, setregid_args *args)
{
    if ( t->thr_proc->p_cred->p_gid == 0 ) {
        if(args->rgid!=-1)
            t->thr_proc->p_cred->p_gid = args->rgid;
        if(args->egid!=-1)
            t->thr_proc->p_cred->p_egid = args->egid;
        r->result = 0;
        return -EOK;
    } else
        return -EPERM;
}

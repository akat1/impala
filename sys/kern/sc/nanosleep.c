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

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/time.h>
#include <sys/utils.h>
#include <sys/syscall.h>

typedef struct sc_nanosleep_args sc_nanosleep_args;

struct sc_nanosleep_args
{
    timespec_t *req;
    timespec_t *rem;
};

errno_t sc_nanosleep(thread_t *p, syscall_result_t *r, sc_nanosleep_args *args );

/**
 * XXX: ///@todo sygna�y i rem
 */

errno_t
sc_nanosleep(thread_t *p, syscall_result_t *r, sc_nanosleep_args *args)
{
    /* sprawdzamy parametry */
    if ( (args->req->tv_sec < 0) || 
         !(args->req->tv_nsec >= 0 && args->req->tv_nsec <= 999999999) )
    {
        return -EINVAL;
    }

    if ( args->req->tv_sec > 0 )
        ssleep(args->req->tv_sec);
    if ( args->req->tv_nsec > 0 )
        msleep(args->req->tv_nsec); // XXX: mili


    return -EOK;
}


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

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>
#include <sys/utils.h>

#include "systab.c"

static int
syscall_invoke(thread_t *thr, syscall_result_t *r, sc_handler_f *handler, ...)
{
    va_list ap;
    VA_START(ap, handler);
    int err = handler(thr, r, ap);
    VA_END(ap);
    return err;
}

void
syscall(thread_t *thr, uint n, syscall_result_t *r, va_list ap)
{
    if (n < SYS_MAX) {
        r->result = 0;
        r->errno = -syscall_table[n](thr, r, ap);
        if (r->errno) {
            r->result = -1;
//            kprintf("Failing syscall %i, errno %i\n", n, r->errno);
        }
    }
    return;
}

void
syscall_raw(thread_t *thr, uint n, syscall_result_t *r,
    uintptr_t arg0, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4, uintptr_t arg5)
{
    if (n < SYS_MAX) {
        r->result = 0;
        r->errno = -syscall_invoke(thr, r, syscall_table[n],
            arg0, arg1, arg2, arg3, arg4, arg5);
        if (r->errno) {
            r->result = -1;
        }
    }
}

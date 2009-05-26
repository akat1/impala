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

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>
#include <sys/utils.h>

errno_t sc_execve(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_exit(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_fork(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_getpid(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_getppid(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_getuid(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_kill(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_lseek(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_nanosleep(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_open(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_pause(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_read(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_setuid(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_sigaction(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_wait(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_waitpid(thread_t *proc, syscall_result_t *r, va_list ap);
errno_t sc_write(thread_t *proc, syscall_result_t *r, va_list ap);

static sc_handler_f *syscall_table[] = {
    sc_execve,
    sc_exit,
    sc_fork,
    sc_getpid,
    sc_getppid,
    sc_getuid,
    sc_kill,
    sc_lseek,
    sc_nanosleep,
    sc_open,
    sc_pause,
    sc_read,
    sc_setuid,
    sc_sigaction,
    sc_wait,
    sc_waitpid,
    sc_write
};

void
syscall(thread_t *thr, int n, syscall_result_t *r, va_list ap)
{
//    kprintf("syscall %u!\n", n);

    if (n < SYS_MAX) {
        r->errno = syscall_table[n](thr, r, ap);
    }

    return;
}


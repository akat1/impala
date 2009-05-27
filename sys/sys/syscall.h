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

#ifndef __SYS_SYSCALL_H
#define __SYS_SYSCALL_H

#include <sys/cdefs.h>

enum {
    EOK = 0
};

enum {
    SYS_execve,
    SYS_exit,
    SYS_fork,
    SYS_getdents,
    SYS_getpid,
    SYS_getppid,
    SYS_getuid,
    SYS_kill,
    SYS_lseek,
    SYS_nanosleep,
    SYS_open,
    SYS_pause,
    SYS_read,
    SYS_setuid,
    SYS_sigaction,
    SYS_wait,
    SYS_waitpid,
    SYS_write,
    SYS_close,
    SYS_link,
    SYS_unlink,
    SYS_symlink,
    SYS_rmdir,
    SYS_dup,
    SYS_dup2,
    SYS_chdir,
    SYS_getgid,
    SYS_setgid,
    SYS_umask,
    SYS_MAX
};

#ifdef __KERNEL

/// struktura s�u��ca do zwracania wynik�w z wywo�a� systemowych
struct syscall_result {
    int result;
    int errno;
};

typedef struct syscall_result syscall_result_t;

void syscall(thread_t *proc, int n, syscall_result_t *r, va_list ap);

typedef errno_t sc_handler_f(thread_t *proc, syscall_result_t *r, va_list ap);

#endif

#endif

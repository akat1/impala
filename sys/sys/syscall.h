/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
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
    SYSCALL_execve,
    SYSCALL_exit,
    SYSCALL_fork,
    SYSCALL_getpid,
    SYSCALL_getppid,
    SYSCALL_getuid,
    SYSCALL_kill,
    SYSCALL_lseek,
    SYSCALL_nanosleep,
    SYSCALL_open,
    SYSCALL_pause,
    SYSCALL_read,
    SYSCALL_setuid,
    SYSCALL_sigaction,
    SYSCALL_wait,
    SYSCALL_waitpid,
    SYSCALL_write,
    SYSCALL_MAX
};

#ifdef __KERNEL

/// struktura s³u¿±ca do zwracania wyników z wywo³añ systemowych
struct syscall_result { 
    int result;
    int errno;
};

typedef struct syscall_result syscall_result_t;

void syscall(thread_t *proc, int n, syscall_result_t *r, va_list ap);

typedef errno_t sc_handler_f(thread_t *proc, syscall_result_t *r, va_list ap);

#endif

#endif

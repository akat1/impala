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
    SYSCALL_exit,
    SYSCALL_open,
    SYSCALL_read,
    SYSCALL_write,
    SYSCALL_close,
    SYSCALL_fork,
    SYSCALL_entry_exec,
    SYSCALL_MAX
};

#ifdef __KERNEL

void syscall(thread_t *p, int n, va_list ap);
#endif

#endif

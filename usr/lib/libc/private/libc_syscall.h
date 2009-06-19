#ifndef __LIBC_SYSCALL_H__
#define __LIBC_SYSCALL_H__

#include <signal.h>

int syscall(int sc,...);
void __sig_handler(int signum);

extern sighandler_t __sig_handlers[NSIG];


#endif

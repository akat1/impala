#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

#include "libc_syscall.h"


int
fcntl(int fd, int cmd, ...)
{
    int arg = 0;
    va_list va;
    va_start(va, cmd);
    arg = va_arg(va, int);
    va_end(va);
    return syscall(SYS_fcntl, fd, cmd, arg);
}

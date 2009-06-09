#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

#include "libc_syscall.h"


int
open(const char *fname, int flags, ...)
{
    mode_t mode=0;
    va_list va;
    va_start(va, flags);
    if(flags & O_CREAT)
        mode = va_arg(va, mode_t);
    va_end(va);
    return syscall(SYS_open, fname, flags, mode);
}

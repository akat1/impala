#include <sys/syscall.h>
#include <fcntl.h>

#include "libc_syscall.h"


int
open(const char *fname, int flags, ...)
{
    mode_t mode=0;
    va_list va;
    VA_START(va, flags);
    if(flags & O_CREAT)
        mode = VA_ARG(va, mode_t);
    return syscall(SYS_open, fname, flags, mode);
    VA_END(va);
}

#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdarg.h>

#include "libc_syscall.h"


int
ioctl(int fd, int cmd, ...)
{
    va_list va;
    va_start(va, cmd);
    uintptr_t param = 0;
    param = va_arg(va, uintptr_t); //no trudno...
    va_end(va);
    return syscall(SYS_ioctl, fd, cmd, param);
}

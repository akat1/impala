#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "libc_syscall.h"


int
ioctl(int fd, int cmd, ...)
{
    va_list va;
    VA_START(va, cmd);
    uintptr_t param = 0;
    param = VA_ARG(va, uintptr_t); //no trudno...
    VA_END(va);
    return syscall(SYS_ioctl, cmd, param);
}

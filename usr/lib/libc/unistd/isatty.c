#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
isatty(int fd)
{
    return syscall(SYS_isatty, fd);
}

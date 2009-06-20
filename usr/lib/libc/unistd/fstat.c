#include <sys/syscall.h>
#include <sys/stat.h>

#include "libc_syscall.h"

int
fstat(int fd, struct stat *buf)
{
    return syscall(SYS_fstat, fd, buf);
}

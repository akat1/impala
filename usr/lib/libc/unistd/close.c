#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"


int
close(int fd)
{
    return syscall(SYS_close, fd);
}

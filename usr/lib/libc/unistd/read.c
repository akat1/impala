#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

ssize_t
read(int fd, void *data, size_t l)
{
    return syscall(SYS_read, fd, data, l);
}

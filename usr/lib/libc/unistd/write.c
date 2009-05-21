#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

ssize_t
write(int fd, const void *data, size_t l)
{
    syscall(SYS_write, fd, data, l);
    return l;
}

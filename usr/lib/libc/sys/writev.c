#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/uio.h>

ssize_t
writev(int fd, const struct iovec *vector, int count)
{
    return syscall(SYS_writev, fd, vector, count);
}


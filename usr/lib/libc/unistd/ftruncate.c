#include <sys/syscall.h>
#include <unistd.h>
#include <libc_syscall.h>

int
ftruncate(int fd, off_t length)
{
    return syscall(SYS_ftruncate, fd, length);
}

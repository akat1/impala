#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

int
fsync(int fd)
{
    return syscall(SYS_fsync, fd);
}

#include <sys/syscall.h>
#include <fcntl.h>

#include "libc_syscall.h"

int
creat(const char *fname, mode_t mode)
{
    return syscall(SYS_open, fname, O_CREAT|O_WRONLY|O_TRUNC, mode);
}

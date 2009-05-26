#include <sys/syscall.h>
#include <fcntl.h>

#include "libc_syscall.h"


int
open(const char *fname, int flags, mode_t mode)
{
    return syscall(SYS_open, fname, flags, mode);
}

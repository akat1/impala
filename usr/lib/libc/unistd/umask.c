#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

mode_t
umask(mode_t mask)
{
    return syscall(SYS_umask, mask);
}

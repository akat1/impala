#include <sys/syscall.h>
#include <sys/stat.h>

#include "libc_syscall.h"

int
stat(const char *path, struct stat *buf)
{
    return syscall(SYS_stat, STAT_NORMAL, path, buf);
}

#include <sys/syscall.h>
#include <sys/stat.h>

#include "libc_syscall.h"

int
lstat(const char *path, struct stat *buf)
{
    return syscall(SYS_stat, STAT_LINK, path, buf);
}
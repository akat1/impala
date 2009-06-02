#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
execve(const char *path, char *const argv[], char *const envp[])
{
    return syscall(SYS_execve, path, argv, envp);
}

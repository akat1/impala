#include <sys/types.h>
#include <sys/syscall.h>

int
chmod(const char *path, mode_t mode)
{
    return syscall(SYS_chmod, path, mode);
}


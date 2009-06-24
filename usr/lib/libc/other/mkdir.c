#include <sys/types.h>
#include <sys/syscall.h>

int
mkdir(const char *pathname, mode_t mode)
{
    return syscall(SYS_mkdir, pathname, mode);
}

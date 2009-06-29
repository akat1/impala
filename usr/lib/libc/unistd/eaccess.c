#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

int
eaccess(const char *pathname, int mode)
{
    return syscall(SYS_access, pathname, mode&15);
}

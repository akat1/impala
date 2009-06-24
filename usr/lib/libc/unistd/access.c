#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

int
access(const char *fname, int mode)
{
    return syscall(SYS_access, fname, mode);
}

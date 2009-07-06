#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

#define __REAL 16

int
access(const char *fname, int mode)
{
    return syscall(SYS_access, fname, (mode&15) | __REAL);
}

#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
setgid(gid_t gid)
{
    return syscall(SYS_setgid, gid);
}

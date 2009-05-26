#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
setuid(uid_t uid)
{
    return syscall(SYS_setuid, uid);
}

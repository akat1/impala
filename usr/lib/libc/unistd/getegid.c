#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

pid_t
getegid(void)
{
    return syscall(SYS_getegid);
}

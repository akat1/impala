#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

pid_t
setsid(void)
{
    return syscall(SYS_setsid);
}

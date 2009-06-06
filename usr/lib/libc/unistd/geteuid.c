#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

pid_t
geteuid(void)
{
    return syscall(SYS_geteuid);
}

#include <sys/wait.h>
#include <sys/syscall.h>

pid_t
wait3(int *status, int options, struct rusage *rusage)
{
    return syscall(SYS_waitpid, -1, status, options);
}

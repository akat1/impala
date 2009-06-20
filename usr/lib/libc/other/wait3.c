#include <sys/wait.h>
#include <sys/syscall.h>
#include <errno.h>

pid_t
wait3(int *status, int options, struct rusage *rusage)
{
    int res = syscall(SYS_waitpid, -1, status, options);
    if(errno == ECHILD && options & WNOHANG)
        return 0;
    return res;
}

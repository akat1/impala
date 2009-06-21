#include <sys/wait.h>
#include <sys/syscall.h>
#include <errno.h>

pid_t
waitpid(pid_t pid, int *status, int options)
{
    int res = syscall(SYS_waitpid, pid, status, options);
    if(errno == ECHILD && options & WNOHANG)
        return 0;
    return res;
}

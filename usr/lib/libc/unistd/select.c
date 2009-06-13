#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>


int
select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout)
{
    return syscall(SYS_select, nfds, readfds, writefds, exceptfds, timeout);
}

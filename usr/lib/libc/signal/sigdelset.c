#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigdelset(sigset_t *set, int signum)
{
    if ( signum < 1 || signum > _NSIG )
        return -1;
    
    *set &= ~sigmask(signum);
    return 0;
}

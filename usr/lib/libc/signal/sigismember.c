#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigismember(const sigset_t *set, int signum)
{
    if ( signum < 1 || signum > _NSIG )
        return -1;
    
    if ( *set & sigmask(signum) )
        return 1;
    else
        return 0;
}

#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

void
__sig_handler(int signum)
{
    (__sig_handlers[signum])(NSIG);
    sigreturn();
}

sighandler_t
signal(int signum, sighandler_t handler)
{
    sighandler_t r;

    if ( signum < 0 || signum >= NSIG )
        return SIG_ERR;

    r = __sig_handlers[signum];

    if ( (sighandler_t)syscall(SYS_signal, signum, __sig_handler) == SIG_ERR )
        return SIG_ERR;

    __sig_handlers[signum] = handler;

    return r;
}

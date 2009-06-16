#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigfillset(sigset_t *set)
{
    sigemptyset(set);
    *set = ~*set;
    return 0;
}

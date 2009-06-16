#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigpending(sigset_t *set)
{
    return syscall(SYS_sigpending, set);
}

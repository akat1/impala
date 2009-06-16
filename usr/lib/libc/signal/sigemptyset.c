#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigemptyset(sigset_t *set)
{
    *set ^= *set;
    return 0;
}

#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigprocmask(int how, const sigset_t *new, sigset_t *old)
{
	return syscall(SYS_sigprocmask, how, new, old);
}

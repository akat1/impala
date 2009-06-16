#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigaction(int signum, const sigaction_t *act, sigaction_t *oldact)
{
	return syscall(SYS_sigprocmask, signum, act, oldact);
}

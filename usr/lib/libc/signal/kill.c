#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

int
kill(pid_t pid, int sig)
{
	return syscall(SYS_kill, sig);
}

#include <sys/syscall.h>
#include <signal.h>
#include <unistd.h>

#include "libc_syscall.h"

int
raise(int sig)
{
	return kill(getpid(), sig);
}

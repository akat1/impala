#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

sighandler_t
signal(int signum, sighandler_t handler)
{
	return (sighandler_t)syscall(SYS_signal, signum, handler);
}

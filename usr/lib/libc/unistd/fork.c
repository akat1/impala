#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

pid_t
fork(void)
{
	return syscall(SYS_fork);
}

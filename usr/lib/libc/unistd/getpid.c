#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

pid_t
getpid(void)
{
	return syscall(SYS_getpid);
}

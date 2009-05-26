#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

pid_t
getppid(void)
{
	return syscall(SYS_getppid);
}

#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

uid_t
getuid(void)
{
	return syscall(SYS_getuid);
}

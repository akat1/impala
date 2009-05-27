#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

gid_t
getgid(void)
{
	return syscall(SYS_getgid);
}

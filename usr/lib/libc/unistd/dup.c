#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
dup(int fd)
{
	return syscall(SYS_dup, fd);
}

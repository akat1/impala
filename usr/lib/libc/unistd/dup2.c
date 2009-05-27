#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
dup2(int oldfd, int newfd)
{
	return syscall(SYS_dup2, oldfd, newfd);
}

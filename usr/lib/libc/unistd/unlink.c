#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
unlink(const char *path)
{
	return syscall(SYS_unlink, path);
}

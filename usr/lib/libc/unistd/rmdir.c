#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
rmdir(const char *path)
{
	return syscall(SYS_rmdir, path);
}
#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
chdir(const char *path)
{
	return syscall(SYS_chdir, path);
}

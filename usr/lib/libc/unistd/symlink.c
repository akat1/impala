#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
symlink(const char *oldpath, const char *newpath)
{
	return syscall(SYS_symlink, oldpath, newpath);
}

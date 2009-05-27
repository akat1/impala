#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
link(const char *oldpath, const char *newpath)
{
	return syscall(SYS_link, oldpath, newpath);
}

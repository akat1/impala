#include <sys/syscall.h>
#include <unistd.h>

#include "libc_syscall.h"

int
getdents(int fd, dirent_t *data, size_t count)
{
	return syscall(SYS_getdents, fd, data, count);
}

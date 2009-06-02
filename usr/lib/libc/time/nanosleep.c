#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "libc_syscall.h"

int
nanosleep(const timespec_t *req, timespec_t *el)
{
	return syscall(SYS_nanosleep, req, el);
}

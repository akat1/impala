#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>

int
mkdir(const char *name, mode_t m)
{
	return syscall(SYS_mkdir, name, m);
}

#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/syscall.h>

int
getmountinfo(int off, struct mountinfo *tab, int n)
{
	return syscall(SYS_getmountinfo, off, tab, n);
}

#include <sys/types.h>
#include <sys/proc.h>
#include <sys/syscall.h>

int
getprocinfo(int off, struct procinfo *tab, int n)
{
	return syscall(SYS_getprocinfo, off, tab, n);
}

#include <sys/types.h>
#include <sys/syscall.h>
#include <sched.h>

int
sched_yield()
{
	return syscall(SYS_sched_yield);
}

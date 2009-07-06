#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>

int
gettimeofday(struct timeval *tp, struct timezone *tz)
{
	return syscall(SYS_gettimeofday, tp, tz);
}

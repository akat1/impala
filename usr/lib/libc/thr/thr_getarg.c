#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

void*
thr_getarg()
{
	return (void*)syscall(SYS_thr_getarg);
}

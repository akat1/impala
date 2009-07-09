#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

int
thr_mtx_wait(mid_t m)
{
	return syscall(SYS_thr_mtx_wait, m);
}


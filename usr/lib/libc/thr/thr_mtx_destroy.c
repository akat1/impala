#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

int
thr_mtx_destroy(mid_t m)
{
	return syscall(SYS_thr_mtx_destroy, m);
}


#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

int
thr_mtx_wakeup_all(mid_t m)
{
	return syscall(SYS_thr_mtx_wakeup_all, m);
}


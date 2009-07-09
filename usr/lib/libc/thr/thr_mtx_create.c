#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

mid_t
thr_mtx_create()
{
	return syscall(SYS_thr_mtx_create);
}


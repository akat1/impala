#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

int
thr_destroy(tid_t tid)
{
	return syscall(SYS_thr_destroy, tid);
}

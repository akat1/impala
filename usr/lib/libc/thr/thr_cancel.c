#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

int
thr_cancel(tid_t tid)
{
	return syscall(SYS_thr_cancel);
}

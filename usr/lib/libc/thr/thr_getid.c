#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

tid_t
thr_getid()
{
	return syscall(SYS_thr_getid);
}

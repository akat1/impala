#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

tid_t
thr_create(void *addr, void *stackaddr, size_t stacksize, void *arg)
{
	return syscall(SYS_thr_create, addr, stackaddr, stacksize, arg);
}

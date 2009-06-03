#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

tid_t
thr_create(void *addr, uintptr_t stackaddr, uintptr_t stacksize, uintptr_t arg)
{
	return syscall(SYS_thr_create, addr, stackaddr, stacksize, arg);
}

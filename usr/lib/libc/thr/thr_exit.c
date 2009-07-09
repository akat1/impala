#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

int
thr_exit()
{
	return syscall(SYS_thr_exit);
}

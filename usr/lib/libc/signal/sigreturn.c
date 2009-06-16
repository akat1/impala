#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

void
sigreturn(void)
{
	syscall(SYS_sigreturn);
    return;
}

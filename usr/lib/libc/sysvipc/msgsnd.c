#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

#include "libc_syscall.h"

int
msgsnd(int id, const void *d, size_t sz, int f)
{
    return syscall(SYS_msgsnd, id, d, sz, f);
}


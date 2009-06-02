#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

#include "libc_syscall.h"

int
msgrcv(int id, void *msg, size_t sz, long type, int f)
{
    return syscall(SYS_msgrcv, id, msg, sz, type, f);
}

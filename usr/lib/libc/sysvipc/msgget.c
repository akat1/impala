#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

#include "libc_syscall.h"

int
msgget(key_t key, int msgflg)
{
    return syscall(SYS_msgget, key, msgflg);
}

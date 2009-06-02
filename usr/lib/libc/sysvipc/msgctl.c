#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

#include "libc_syscall.h"

int
msgctl(int id, int cmd, struct msqid_ds *buf)
{
    return syscall(SYS_msgctl, id, cmd, buf);
}

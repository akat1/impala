#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

int
setpgrp(void)
{
    return setpgid(0, 0);
}

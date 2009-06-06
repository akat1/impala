#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

pid_t
getpgrp(void)
{
    return getpgid(0);
}

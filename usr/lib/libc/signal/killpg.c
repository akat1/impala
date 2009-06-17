#include <sys/syscall.h>
#include <signal.h>

#include "libc_syscall.h"

int
killpg(int pgrp, int sig)
{
    return kill(-pgrp, sig); //tak tymczasowo
}

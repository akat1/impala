#include <sys/syscall.h>
#include <time.h>

#include <unistd.h>

#include "libc_syscall.h"

unsigned int
sleep(unsigned int secs)
{
    timespec_t req;
    timespec_t el;
    req.tv_sec = secs;
    req.tv_nsec = 0;
    if (nanosleep(&req, &el)) return -1;
    return el.tv_sec;
}

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

static char __ttyname_buf[128];

char *
ttyname(int fd)
{
    if(syscall(SYS_ttyname, fd, __ttyname_buf, 128))
        return NULL;
    return __ttyname_buf;
}

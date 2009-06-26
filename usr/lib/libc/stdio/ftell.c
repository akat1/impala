#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>

long
ftell(FILE *stream)
{
    if(!stream || ISUNSET(stream->status, _FST_OPEN)) {
        errno = EBADF;
        return -1;
    }
    fflush(stream);
    return syscall(SYS_ftell, stream->fd);
}

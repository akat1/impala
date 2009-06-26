#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>

int
fseek(FILE *stream, long offset, int whence)
{
    if(!stream || ISUNSET(stream->status, _FST_OPEN)) {
        errno = EBADF;
        return -1;
    }
    fflush(stream);
    int res = syscall(SYS_fseek, stream->fd, offset, whence);
    UNSET(stream->err, _FER_EOF);
    return res;
}

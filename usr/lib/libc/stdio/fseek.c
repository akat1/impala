#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>

int
fseek(FILE *stream, long offset, int whence)
{
    int res = syscall(SYS_fseek, stream->fd, offset, whence);
    UNSET(stream->err, _FER_EOF);
    return res;
}

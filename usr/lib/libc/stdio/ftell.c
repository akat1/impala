#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>

long
ftell(FILE *stream)
{
    return syscall(SYS_ftell, stream->fd);
}

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fflush(FILE *stream)
{
    if(ISUNSET(stream->status,_FST_OPEN)) {
        //errno = -EBADF;
        return -1;
    }
    if(ISSET(stream->status, _FST_NOBUF))
        return 0;
    return 0; //todo ;)
    //return write(stream->fd, stream->buf, 0);
}

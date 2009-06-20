#include <stdio.h>
#include <stdio_private.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fflush(FILE *stream)
{
    if(ISUNSET(stream->status,_FST_OPEN)) {
        errno = EBADF;
        return EOF;
    }
    if(ISSET(stream->status, _FST_NOBUF))
        return 0;
    __check_buf(stream);
    int res = 0;
    int beg = 0;
    while(stream->inbuf > 0) {
        if(stream->writefn)
            res = stream->writefn(stream->cookie, stream->buf+beg, stream->inbuf); 
        else if(stream->fd != -1)
            res = write(stream->fd, stream->buf+beg, stream->inbuf);
        if(res <= 0)
            return EOF;
        beg += res;
        stream->inbuf -= res;
    }
    return 0;
}

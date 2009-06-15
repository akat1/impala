#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fclose(FILE *stream)
{
    int ret;
    ret = fflush(stream);
    if(ret) {
        UNSET(stream->status, _FST_OPEN);
        return ret;
    }
    if(stream->closefn)
        ret = stream->closefn(stream->cookie);
    else
        ret = close(stream->fd);
    free(stream);
    return ret;
}

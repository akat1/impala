#include <stdio.h>
#include <stdio_private.h>
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
    list_remove(&__open_files, stream);
    if(stream->closefn)
        ret = stream->closefn(stream->cookie);
    else if(stream->fd!=-1)
        ret = close(stream->fd);
    stream->status = 0;
    free(stream);
    return ret;
}

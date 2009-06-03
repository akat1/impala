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
    // fflush
    ret = close(stream->fd);
    free(stream);
    return ret;
}

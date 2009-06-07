#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fgetc(FILE * f)
{
    if(ISUNSET(f->status, _FST_OPEN))
        return EOF;
    char ch[2];
    int ret = read(f->fd, ch, 1);
    if(ret<=0)
        return EOF;
    return (int)ch[0];
}

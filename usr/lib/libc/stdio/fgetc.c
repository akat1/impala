#include <stdio.h>
#include <stdio_private.h>
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
    int ret;
    __check_buf(f);
    if(ISSET(f->status, _FST_TTY))
        __fflush_line_buffered();
    if(f->readfn)
        ret = f->readfn(f->cookie, ch, 1);
    else
        ret = read(f->fd, ch, 1);
    if(ret<=0)
        return EOF;
    return (int)ch[0];
}

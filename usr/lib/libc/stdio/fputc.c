#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fputc(int ch, FILE * f)
{
    if(ISUNSET(f->status,_FST_OPEN))
        return EOF;
    unsigned char c = (unsigned char)ch;
    if(ISSET(f->status, _FST_NOBUF))
        return write(f->fd, &c, 1);
    return 0;//todo..
}

int
putc(int ch, FILE * f)
{
    return fputc(ch, f);
}


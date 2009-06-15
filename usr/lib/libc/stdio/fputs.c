#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int
fputs(const char *str, FILE *f)
{
    if(ISUNSET(f->status,_FST_OPEN))
        return EOF;
    if(ISSET(f->status, _FST_NOBUF)) {
        if(f->writefn)
            return f->writefn(f->cookie, str, strlen(str));
        return write(f->fd, str, strlen(str));
    }
    return 0;//todo..
}



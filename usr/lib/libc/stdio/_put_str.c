#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio_private.h>

int
__put_str(FILE *f, const char *str)
{
    int res = 0;
    if(ISSET(f->status, _FST_NOBUF)) {
        if(f->writefn) {
            res = f->writefn(f->cookie, str, strlen(str));
            if(res <= 0)
                return EOF;
        } else if(f->fd!=-1) {
            res = write(f->fd, str, strlen(str));
            if(res <= 0)
                return EOF;
        } else
            return EOF;
        return res;
    }
    while(*str) {
        char c = *(str++);
        bool lineBuf = ISSET(f->status, _FST_LINEBUF);
        f->buf[f->inbuf++] = c;
        if(f->inbuf == f->buf_size || (c == '\n' && lineBuf))
            fflush(f);
        res++;
    }
    return res;
}

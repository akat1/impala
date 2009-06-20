#include <stdio.h>
#include <unistd.h>
#include <stdio_private.h>

int
__put_char(FILE *f, char c)
{
    int res = 0;
    if(ISSET(f->status, _FST_NOBUF)) {
        if(f->writefn) {
            res = f->writefn(f->cookie, &c, 1);
            if(res <= 0)
                return EOF;
        } else if(f->fd!=-1) {
            res = write(f->fd, &c, 1);
            if(res <= 0)
                return EOF;
        }
    } else if(ISSET(f->status, _FST_LINEBUF)) {
        f->buf[f->inbuf++] = c;
        if(f->inbuf == f->buf_size || c == '\n')
            fflush(f);
    } else if(ISSET(f->status, _FST_FULLBUF)) {
        f->buf[f->inbuf++] = c;
        if(f->inbuf == f->buf_size)
            fflush(f);
    }
    return c;
}
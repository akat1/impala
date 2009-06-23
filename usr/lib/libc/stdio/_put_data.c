#include <stdio.h>
#include <unistd.h>
#include <stdio_private.h>

int
__put_data(FILE *f, const char *str, size_t size)
{
    int res = 0;
    if(ISSET(f->status, _FST_NOBUF)) {
        if(f->writefn) {
            res = f->writefn(f->cookie, str, size);
        } else if(f->fd!=-1) {
            res = write(f->fd, str, size);
        } else
            return EOF;
        if(res <= 0)
            return EOF;
        return res;
    }
    bool lineBuf = ISSET(f->status, _FST_LINEBUF);
    while(size-- > 0) {
        char c = *(str++);
        f->buf[f->inbuf++] = c;
        if(f->inbuf == f->buf_size || (c == '\n' && lineBuf))
            fflush(f);
        res++;
    }
    return res;
}
#include <stdio.h>
#include <unistd.h>
#include <stdio_private.h>

int
__get_data(FILE *f, char *str, size_t size)
{
    int res = 0;
    if(f->writefn) {
        res = f->readfn(f->cookie, str, size);
        if(res <= 0)
            return EOF;
    } else if(f->fd!=-1) {
        res = read(f->fd, str, size);
        if(res <= 0)
            return EOF;
        } else
            return EOF;
    return res;
}

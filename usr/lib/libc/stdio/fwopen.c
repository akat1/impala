#include <stdio.h>
#include <stdio_private.h>
#include <stdlib.h>
#include <sys/list.h>

FILE *
fwopen(void *cookie, int (*writefn)(void *, const char *, int))
{
    FILE *f = malloc(sizeof(FILE));
    if(!f)
        return NULL;
    f->fd = -1;
    f->cookie = cookie;
    f->writefn = writefn;
    f->readfn = NULL;
    f->seekfn = NULL;
    f->closefn = NULL;
    f->buf = NULL;
    f->buf_size = BUFSIZ;
    f->inbuf = 0;
    f->status = _FST_OPEN | _FST_NOBUF;
    list_insert_tail(&__open_files, f);
    return f;
}

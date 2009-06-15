#include <stdio.h>
#include <stdlib.h>

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
    f->status = _FST_OPEN | _FST_NOBUF;
    return f;
}

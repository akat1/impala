#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio_private.h>


FILE *
fopen(const char *path, const char *mode)
{
    if(!mode || !path) {
        errno = EINVAL;
        return NULL;
    }
    FILE *f = malloc(sizeof(FILE));
    if(!f)
        return NULL;
    f->fd = -1;
    f->cookie = NULL;
    f->writefn = NULL;
    f->readfn = NULL;
    f->seekfn = NULL;
    f->closefn = NULL;
    f->buf = NULL;
    f->buf_size = BUFSIZ;
    f->inbuf = 0;
    int flags = 0;
    if(*mode == 'r')
        flags |= O_RDONLY;
    else if(*mode == 'w')
        flags |= O_TRUNC | O_CREAT | O_WRONLY;
    else if(*mode == 'a')
        flags |= O_APPEND | O_CREAT | O_WRONLY;
    if(strchr(mode, '+'))
        flags = (flags & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
    f->err = 0;
    f->status = 0;
    f->fd = open(path, flags, 0666);
    if(f->fd == -1) {
        free(f);
        return NULL;
    }
    f->status = _FST_OPEN;
    if(isatty(f->fd))
        f->status |= _FST_TTY | _FST_LINEBUF;
    else
        f->status |= _FST_FULLBUF;
    list_insert_tail(&__open_files, f);
    return f;
}
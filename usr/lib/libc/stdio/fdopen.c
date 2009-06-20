#include <stdio.h>
#include <stdio_private.h>
#include <errno.h>
#include <unistd.h>
#include <sys/list.h>

FILE *
fdopen(int fd, const char *mode)
{
    if(!mode || fd<0) {
        errno = EINVAL;
        return NULL;
    }
    FILE *f = malloc(sizeof(FILE));
    if(!f)
        return NULL;
    f->fd = fd;
    f->cookie = NULL;
    f->writefn = NULL;
    f->readfn = NULL;
    f->seekfn = NULL;
    f->closefn = NULL;
    f->buf = NULL;
    f->buf_size = BUFSIZ;
    f->inbuf = 0;
//    int flags = 0;
//     if(*mode == 'r')
//         flags |= O_RDONLY;
//     else if(*mode == 'w')
//         flags |= O_TRUNC | O_CREAT | O_WRONLY;
//     else if(*mode == 'a')
//         flags |= O_APPEND | O_CREAT | O_WRONLY;
//     if(strchr(mode, '+'))
//         flags = (flags & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
    f->err = 0;
    f->status = _FST_OPEN | _FST_NOBUF;
    if(isatty(fd))
        f->status |= _FST_TTY;
    list_insert_tail(&__open_files, f);
    return f;
}

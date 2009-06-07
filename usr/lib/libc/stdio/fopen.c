#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

FILE _stdF[3] = {{.fd=0}, {.fd=1}, {.fd=2}};

FILE *
fopen(const char *path, const char *mode)
{
    if(!mode || !path) {
        errno = -EINVAL;
        return NULL;
    }
    FILE *f = malloc(sizeof(FILE));
    if(!f)
        return NULL;
    f->fd = -1;
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
    f->fd = open(path, flags, 0666);
    if(f->fd == -1) {
        free(f);
        return NULL;
    }
    return f;
}
#include <stdio.h>
#include <stdio_private.h>

int
fileno(FILE *f)
{
    return f->fd;
}
#include <stdio.h>
#include <stdio_private.h>
#include <unistd.h>

void
__check_buf(FILE *f)
{
    if(!f || f->buf)
        return;
    if(f->buf_size == 0)
        return;
    f->buf = malloc(f->buf_size);
}

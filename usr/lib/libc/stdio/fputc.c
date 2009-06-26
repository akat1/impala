#include <stdio.h>
#include <stdio_private.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fputc(int ch, FILE * f)
{
    if(!f || ISUNSET(f->status,_FST_OPEN))
        return EOF;
    __check_buf(f);
    return __put_char(f, ch);
}

int
putc(int ch, FILE * f)
{
    return fputc(ch, f);
}


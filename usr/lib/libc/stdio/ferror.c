#include <stdio.h>
#include <stdio_private.h>

int
ferror(FILE *f)
{
    return (f->err & _FER_ERR)>0;
}

#include <stdio.h>

int
ferror(FILE *f)
{
    return (f->err & _FER_ERR)>0;
}

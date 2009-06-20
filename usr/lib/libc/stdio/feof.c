#include <stdio.h>

int
feof(FILE *f)
{
    return (f->err & _FER_EOF)>0;
}

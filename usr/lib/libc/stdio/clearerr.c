#include <stdio.h>

void
clearerr(FILE *f)
{
    f->err = 0;
}

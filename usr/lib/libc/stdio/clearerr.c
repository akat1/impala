#include <stdio.h>
#include <stdio_private.h>

void
clearerr(FILE *f)
{
    f->err = 0;
}

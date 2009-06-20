#include <stdio.h>
#include <stdio_private.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int
fputs(const char *str, FILE *f)
{
    if(ISUNSET(f->status, _FST_OPEN))
        return EOF;
    __check_buf(f);
    return __put_str(f, str);
}



#include <stdio.h>
#include "stdio_private.h"
#include <unistd.h>

//@todo dopracowa�
size_t
fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    int i = 0;
    int r;
    __check_buf(stream);
    fflush(stream);
    while( nmemb-- ) {
        r = __get_data(stream, ptr, size);
        if (r == size) {
            i++;
        } else return i;
    }
    return i;
}

#include <stdio.h>
#include <stdio_private.h>
#include <unistd.h>

size_t
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if(ISUNSET(stream->status,_FST_OPEN))
        return 0;
    __check_buf(stream);
    int res = __put_data(stream, ptr, size*nmemb);
    if(res>=0)
        return res;
    return 0;
}

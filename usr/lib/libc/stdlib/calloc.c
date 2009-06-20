#include <stdlib.h>
#include <string.h>

void *
calloc(size_t nmemb, size_t size)
{
    void* res = malloc(nmemb*size);
    memset(res, 0, nmemb*size);
    return res;
}

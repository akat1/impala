#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void*
realloc(void *ptr, size_t size)
{
    ///@todo impl
    if(!ptr)
        return malloc(size);
    if(!size) {
        free(ptr);
        return NULL;
    }
    size_t oldsize = ((int*)ptr)[-1];
    if(size<oldsize)
    {
        ((int*)ptr)[-1] = size;//mo¿na by zwolniæ to wy¿ej ;)
        return ptr;
    }
    void* naddr = malloc(size);
    if(!naddr) {
        errno = ENOMEM;
        return NULL;
    }
    memcpy(naddr, ptr, oldsize);
    free(ptr);
    return naddr;
}
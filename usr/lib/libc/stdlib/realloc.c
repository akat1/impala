#include <stdlib.h>
#include <stdlib_private.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void*
realloc(void *ptr, size_t size)
{
    ///@todo poprawiæ realloc
    if(!ptr)
        return malloc(size);
    if(!size) {
        free(ptr);
        return NULL;
    }
    mem_chunk_t *mc = ptr - sizeof(mem_chunk_t);
    size_t oldsize = mc->size;
    if(size<oldsize)
    {
        //mo¿na zwalniaæ nadmiarow± pam..
        return ptr;
    }
    //naiwna implementacja
    void* naddr = malloc(size);
    if(!naddr) {
        errno = ENOMEM;
        return NULL;
    }
    memcpy(naddr, ptr, oldsize);
    free(ptr);
    return naddr;
}
#include <sys/types.h>
#include <sys/list.h>
#include <stdlib_private.h>
#include <stdlib.h>
#include <unistd.h>


static bool __find_free_chunk(const void *chunk, uint arg);
static list_less_f __less_f;

bool
__find_free_chunk(const void *chunk, uint arg)
{
    if ( ((mem_chunk_t *)chunk)->size >= arg && ((mem_chunk_t *)chunk)->avail )
        return TRUE;
    else
        return FALSE;
}

bool
__less_f(const void *x, const void *y)
{
    return x < y;
}


void*
malloc(size_t size)
{
    return sbrk(size)-size;
    int ns = size+sizeof(mem_chunk_t);
    char *addr;
    mem_chunk_t *mc,*nc;

    mc = list_find(&__mem_chunks, __find_free_chunk, size);
    
    if ( mc == NULL ) {
        addr = sbrk(ns)-ns;
        mc = (mem_chunk_t *)addr;
        addr += sizeof(mem_chunk_t);
        mc->addr = addr;
        mc->avail = FALSE;
        mc->size = size;
        list_insert_in_order(&__mem_chunks, mc, __less_f);
        return addr;
    } else {
        mc->avail = FALSE;
        return mc->addr;
        if ( mc->size-sizeof(mem_chunk_t) <= size ) {
            mc->avail = FALSE;
            return mc->addr;
        } else {
            nc = mc->addr+size;
            nc->size = mc->size-size-sizeof(mem_chunk_t);
            mc->size = size;
            nc->avail = FALSE;
            nc->addr = nc + sizeof(mem_chunk_t);
            list_insert_after(&__mem_chunks, mc, nc);
            return mc->addr;
        }
    }

    return NULL;
}

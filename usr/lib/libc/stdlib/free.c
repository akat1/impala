#include <sys/types.h>
#include <sys/list.h>
#include <stdlib.h>
#include <stdlib_private.h>

bool __find_chunk(const void *chunk, addr_t arg);

bool
__find_chunk(const void *chunk, addr_t arg)
{
    if ( ((mem_chunk_t *)chunk)->size+((mem_chunk_t *)chunk)->addr >= arg && 
         ((mem_chunk_t *)chunk)->addr <= arg )
        return TRUE;
    else
        return FALSE;
}  

void
free(void *ptr)
{
    mem_chunk_t *mc;
    mem_chunk_t *nc;

    mc = list_find(&__mem_chunks, __find_chunk, ptr);
    
    if ( mc == NULL )
        return;

    if ( mc->avail )
        return;

    mc->avail = TRUE;

    /* z��czamy s�siednie chunki */
    nc = list_next(&__mem_chunks, mc);
    
    if ( nc && nc->avail ) {
        mc->size = mc->size+nc->size+sizeof(mem_chunk_t);
        list_remove(&__mem_chunks, nc);
    }
    nc = list_prev(&__mem_chunks, mc);
    if ( nc && nc->avail ) {
        nc->size = nc->size+mc->size+sizeof(mem_chunk_t);
        list_remove(&__mem_chunks, mc);
    } 
    
    return;
}

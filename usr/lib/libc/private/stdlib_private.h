#ifndef __STDLIB_PRIVATE_H__
#define __STDLIB_PRIVATE_H__

#include <sys/types.h>
#include <sys/list.h>

struct mem_chunk {
    addr_t addr;
    bool avail;
    size_t size;
    list_node_t L_mem_chunks;
};

typedef struct mem_chunk mem_chunk_t;

extern list_t __mem_chunks;
#endif

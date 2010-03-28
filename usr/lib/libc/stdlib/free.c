/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest objęty licencją, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#include <sys/types.h>
#include <sys/list.h>
#include <stdlib.h>
#include <stdlib_private.h>

void
free(void *ptr)
{
    return;
    if ( ptr == NULL )
        return;
    mem_chunk_t *mc;
    mem_chunk_t *nc;
    mc = ptr - sizeof(mem_chunk_t);

    if ( mc->avail ) {
        fprintf(stderr, "freeing bad addr %p\n", ptr);
        return;
    }

    mc->avail = TRUE;

    /* złączamy sąsiednie chunki */
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

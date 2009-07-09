/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest objêty licencj±, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#include <sys/types.h>
#include <string.h>

void *
memmove(void *dst, const void *src, size_t len)
{
    void *org = dst;
    int i;

    if ( dst != src && len > 0 ) {
        if (dst < src) {
            for ( i = 0 ; i < len ; i++ )
                *((char *)dst+i) = *((char *)src+i);
        } else {
            for ( i = len - 1 ; i >= 0 ; i-- )
                *((char *)dst+i) = *((char *)src+i);
        }
    }
    return org;
}

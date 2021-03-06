/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
 * All rights reserved.
 *
 * Niniejszy plik jest objęty licencją, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define BASE(X)         ((char *)base + size*(X))

void *
binary_search(const void *key, const void *base, size_t size, 
        int (*compar)(const void *, const void *),int lo, int hi);

void *
binary_search(const void *key, const void *base, size_t size, 
        int (*compar)(const void *, const void *),int lo, int hi)
{
    int mid = (lo+hi)/2;
    int result;

    result = compar(key, BASE(mid));

    if ( result == 0 ) {
        return BASE(mid);
    }

    if (lo == hi)
        return NULL;

    if ( result < 0 ) {
        return binary_search(key, base, size, compar, lo, mid);
     } else {
        return binary_search(key, base, size, compar, mid+1, hi);
    }
}


void *
bsearch(const void *key, const void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *))
{
    return binary_search(key, base, size, compar, 0, nmemb-1); 
}

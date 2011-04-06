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
#include <stdlib.h>
#include <string.h>

void *
calloc(size_t nmemb, size_t size)
{
    void* res;
    /* detect overflow */
    if (nmemb * size < size && nmemb * size < nmemb)
        return NULL;
    res = malloc(nmemb*size);
    if (res == NULL)
        return NULL;
    memset(res, 0, nmemb*size);
    return res;
}

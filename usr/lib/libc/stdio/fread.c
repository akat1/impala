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
#include <stdio.h>
#include "stdio_private.h"
#include <unistd.h>

//@todo dopracowaæ
size_t
fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    int i = 0;
    int r;
    __check_buf(stream);
    fflush(stream);
    r = __get_data(stream, ptr, size*nmemb);
    if(r>0)
        return r/size;
    stream->err |= _FER_EOF;
    return 0;
    while( nmemb-- ) {
        r = __get_data(stream, ptr, size);
        if (r == size) {
            i++;
            ptr+=size;
        } else {
            return i;
        }
    }
    return i;
}

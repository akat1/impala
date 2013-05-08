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
#include <stdio.h>
#include <stdio_private.h>
#include <unistd.h>

size_t
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if(ISUNSET(stream->status,_FST_OPEN))
        return 0;
    __check_buf(stream);
    int res = __put_data(stream, ptr, size*nmemb);
    if(res>=0)
        return res/size;
    return 0;
}

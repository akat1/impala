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
#include <unistd.h>

char *
rindex(const char *s, int c)
{
    char *last = NULL;

    do 
    {
        if ( *s ==  (unsigned char)c )
            last = (char *)s;
    }
    while ( *(s++) );

    return last;
}

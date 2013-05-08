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
#include <string.h>
#include <sys/types.h>

char *
strncat(char *dest, const char *src, size_t n)
{
    char *r = dest;

    while(*dest) {
        dest++;
    }

    while(n--) {
        if ( *src )
            break;

        *(dest++) = *(src++);
    }

    *dest = '\0';

    return r;
}

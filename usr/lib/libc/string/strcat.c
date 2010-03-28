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
#include <string.h>
#include <sys/types.h>

char *
strcat(char *dest, const char *src)
{
    char *r = dest;

    while(*dest) { 
        dest++;
    }

    while(*src) {
        *(dest++) = *(src++);
    }

    *dest = '\0';

    return r;
}

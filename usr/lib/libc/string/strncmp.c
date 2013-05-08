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

int
strncmp(const char *s1, const char *s2, size_t n)
{
    while(n--) {
        if ( *s1 != *s2 ) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }

    return 0;
}

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
    
int
memcmp(const void *s1, const void *s2, size_t n)
{
    uchar *p1 = (uchar *)s1, *p2 = (uchar *)s2;
    
    while(n--) {
        if ( *p1 != *p2 ) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }

    return 0;
}

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
#include <string.h>
#include <ctype.h>

int
strcasecmp(const char *s1, const char *s2)
{
    while ( tolower(*s1) == tolower(*s2) ) {
        if ( *s1 == 0 )
            return 0;
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

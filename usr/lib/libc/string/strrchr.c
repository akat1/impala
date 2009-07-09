/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Niniejszy plik jest obj�ty licencj�, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
#include <string.h>
#include <sys/types.h>

char *
strrchr(const char *s, int c)
{
    char *last = NULL;

    for (; *s ; s++ ) {
        if ( *s == c ) {
            last = (char *)s;
        }
    }

    return last;
}

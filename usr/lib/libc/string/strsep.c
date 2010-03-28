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

char *
strsep(char **stringp, const char *delim)
{
    if(!stringp)
        return NULL;
    char *res = *stringp;
    *stringp = strpbrk(*stringp, delim);
    if(*stringp) {
        **stringp = '\0';
        *stringp = (*stringp)+1;
    }
    return res;
}

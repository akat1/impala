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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

long
atol(const char *nptr)
{
    long res = 0, sign;
    while(isspace(*nptr))
        nptr++;
    if(*nptr == '-') {
        nptr++;
        sign = -1;
    } else {
        if(*nptr == '+')
            nptr++;
        sign = 1;
    }
    while(*nptr) {
        if(isdigit(*nptr))
            res = res*10 + *nptr-'0';
        else break;
        nptr++;
    }
    return sign*res;
}

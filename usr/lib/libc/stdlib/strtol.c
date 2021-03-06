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
#include <sys/errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "libc_syscall.h"

extern int errno;

long int
strtol(const char *nptr, char **endptr, int base)
{
    char *s = (char *)nptr;
    long int result = 0, sign = 1;
    int tmp;

    if (!(base == 0 || (base >= 2 && base <= 36))) {
        errno = EINVAL;
        return 0;
    }

    /* nie bierzemy pod uwagę białych znaków z początku */
    while(isspace(*s)) s++;

    /* prefix */
    if (*s == '-' || *s == '+') {
        if ( *s == '-' ) {
            sign = -1;
        }
        s++;
    }

    if ( base == 0 || base == 16 ) {
        if (s[0] == '0' && ( tolower(s[1]) == 'x' ))
        {
            s += 2;
            base = 16;
        }
    }

    if ( base == 0 && *s == '0' ) {
        base = 8;
    }

    if ( base == 0 )
        base = 10;

    while(*s)
    {
        if ( isdigit(*s) )
            tmp = *s - '0';
        if ( isalpha(*s) )
            tmp = tolower(*s) - 'a';
        if ( tmp >= base )
            break;
        s++;
        result = result * base + tmp;
    }

    if ( endptr ) {
        *endptr = (char *)nptr;
    }

    return sign*result;

}

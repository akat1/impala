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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *
getenv(const char *name)
{
    char **ptr = environ;
    size_t nlen = strlen(name);
    while(*ptr) {
        if(!strncmp(name, *ptr, nlen) && (*ptr)[nlen]=='=')
            return &(*ptr)[nlen+1];
        ptr++;
    }
    return NULL;
}

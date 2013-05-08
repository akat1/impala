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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
extern char **environ;

static char *get_espace(size_t s);

char *
get_espace(size_t s)
{
    ///jak zmienić? co z alokacją / zwalnianiem? póki co naiwnie:
    return malloc(s); ///nie powinno tak być..?
}

int
setenv(const char *name, const char *value, int overwrite)
{
    if(strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }
    char **e = environ;
    size_t nlen = strlen(name);
    size_t dlen = nlen + strlen(value) + 2;
    while(*e) {
        if(!strncmp(*e, name, nlen) && (*e)[nlen]=='=') {
            if(!overwrite)
                return 0;

            char *new_entry = get_espace(dlen);
            sprintf(new_entry, "%s=%s", name, value);
            *e = new_entry; //co ze starą informacją?
            return 0;
        }
        e++;
    }
    //ok, pusto..
    char *new_entry = get_espace(dlen);
    sprintf(new_entry, "%s=%s", name, value);
    *e = new_entry; //co ze starą informacją?
    *(++e) = NULL; // a co jak się nie mieścimy już... jakie duże jest environ?
    return 0;
}

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
#include <stdio.h>
#include <stdio_private.h>
#include <stdlib.h>

FILE *
fwopen(void *cookie, int (*writefn)(void *, const char *, int))
{
    FILE *f = malloc(sizeof(FILE));
    if(!f)
        return NULL;
    f->fd = -1;
    f->cookie = cookie;
    f->writefn = writefn;
    f->readfn = NULL;
    f->seekfn = NULL;
    f->closefn = NULL;
    f->buf = NULL;
    f->buf_size = BUFSIZ;
    f->inbuf = 0;
    f->status = _FST_OPEN | _FST_FULLBUF;
    list_insert_tail(&__open_files, f);
    return f;
}

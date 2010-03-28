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
#include <stdio.h>
#include <unistd.h>
#include <stdio_private.h>

int
__get_data(FILE *f, char *str, size_t size)
{
    int res = 0;
    if(f->readfn) {
        res = f->readfn(f->cookie, str, size);
        if(res <= 0)
            return EOF;
    } else if(f->fd!=-1) {
        res = read(f->fd, str, size);
        if(res <= 0)
            return EOF;
    } else
        return EOF;
    return res;
}

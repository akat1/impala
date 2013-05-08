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
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fgetc(FILE * f)
{
    if(!f || ISUNSET(f->status, _FST_OPEN))
        return EOF;
    char ch[2];
    int ret;
    __check_buf(f);
    if(ISSET(f->status, _FST_TTY))
        __fflush_line_buffered();
    if(f->readfn)
        ret = f->readfn(f->cookie, ch, 1);
    else
        ret = read(f->fd, ch, 1);
    if(ret<=0) {
        f->err |= _FER_EOF;
        return EOF;
    }
    return (int)ch[0];
}

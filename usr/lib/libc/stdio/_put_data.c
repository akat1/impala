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
#include <stdio.h>
#include <unistd.h>
#include <stdio_private.h>

int
__put_data(FILE *f, const char *str, size_t size)
{
    int res = 0;
    if(ISSET(f->status, _FST_NOBUF)) {
        if(f->writefn) {
            res = f->writefn(f->cookie, str, size);
        } else if(f->fd!=-1) {
            res = write(f->fd, str, size);
        } else
            return EOF;
        if(res <= 0)
            return EOF;
        return res;
    }
    bool lineBuf = ISSET(f->status, _FST_LINEBUF);
    while(size-- > 0) {
        char c = *(str++);
        f->buf[f->inbuf++] = c;
        if(f->inbuf == f->buf_size || (c == '\n' && lineBuf))
            fflush(f);
        res++;
    }
    return res;
}
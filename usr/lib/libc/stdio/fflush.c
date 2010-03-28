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
#include <stdio_private.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
fflush(FILE *stream)
{
    if(stream == NULL) {
        printf("Fflushing NULL not implemented\n");
        return 0;
    }
    if(ISUNSET(stream->status,_FST_OPEN)) {
        errno = EBADF;
        return EOF;
    }
    if(ISSET(stream->status, _FST_NOBUF))
        return 0;
    if(stream->buf == NULL)
        return 0;
    //__check_buf(stream); //na pewno chcemy to tu?
    int res = 0;
    int beg = 0;
    while(stream->inbuf > 0) {
        if(stream->writefn)
            res = stream->writefn(stream->cookie, stream->buf+beg, stream->inbuf); 
        else if(stream->fd != -1)
            res = write(stream->fd, stream->buf+beg, stream->inbuf);
        if(res <= 0) {
            stream->inbuf = 0;//żeby nie przepełniło się.. :/
            return EOF;
        }
        beg += res;
        stream->inbuf -= res;
    }
    return 0;
}

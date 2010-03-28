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
fclose(FILE *stream)
{
    if(!stream || ISUNSET(stream->status, _FST_OPEN)) {
        errno = EBADF;
        return EOF;
    }
    int ret = fflush(stream);
    if(ret) {
        UNSET(stream->status, _FST_OPEN);
        return ret;
    }
    list_remove(&__open_files, stream);
    if(stream->closefn)
        ret = stream->closefn(stream->cookie);
    else if(stream->fd!=-1)
        ret = close(stream->fd);
    stream->status = 0;
    for(int i=0; i<3; i++)
        if(_stdF[i]==stream)
            _stdF[i]=NULL;
    free(stream);
    return ret;
}

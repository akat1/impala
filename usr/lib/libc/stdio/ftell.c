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
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdio_private.h>
#include <errno.h>

long
ftell(FILE *stream)
{
    if(!stream || ISUNSET(stream->status, _FST_OPEN)) {
        errno = EBADF;
        return -1;
    }
    fflush(stream);
    return syscall(SYS_ftell, stream->fd);
}

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
#include <stdio_private.h>

int
setvbuf(FILE *f, char *b, int mode, size_t size)
{
    if(f->buf != NULL)
        return -1;  //ju¿ nie mo¿na zmieniaæ -> kto¶ wykona³ I/O na f
    if(mode == _IONBF) {
        UNSET(f->status, _FST_NOBUF | _FST_LINEBUF | _FST_FULLBUF);
        SET(f->status, mode);
        return 0;
    }
    if(mode == _IOFBF || mode == _IOLBF) {
        UNSET(f->status, _FST_NOBUF | _FST_LINEBUF | _FST_FULLBUF);
        SET(f->status, mode);
        f->buf = b; //nawet jak NULL, to OK
        f->buf_size = size;
        return 0;
    }
    return -1;
}
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

void
__fflush_line_buffered(void)
{
    FILE *f = NULL;
    while((f = list_next(&__open_files, f))) {
        if(ISSET(f->status, _FST_LINEBUF) && ISSET(f->status, _FST_OPEN))
            fflush(f);
    }
}

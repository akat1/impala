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
#include <string.h>

char *
fgets(char *s, int size, FILE *f)
{
    if(!f || ISUNSET(f->status,_FST_OPEN))
        return NULL;
    __check_buf(f);
    if(ISSET(f->status, _FST_TTY))
        __fflush_line_buffered();
    char *c = s;
    *c = 'A';
    int cnt=0, res=0;
    if(f->readfn) {
        while(cnt++ < size-1 && *c!=EOF && *c!='\n')
            res = f->readfn(f->cookie, c++, 1);

    } else if(f->fd!=-1) {
        while(cnt++ < size-1 && *c!=EOF && *c!='\n')
            res = read(f->fd, c++, 1);
    }
    if(res == 0)    //EOF
        cnt--;
    s[cnt] = '\0';
    if(cnt <= 0) {
        f->err |= _FER_EOF;
        return NULL;
    }
    return s;
}



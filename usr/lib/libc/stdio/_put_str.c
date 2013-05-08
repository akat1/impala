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
#include <unistd.h>
#include <string.h>
#include <stdio_private.h>

int
__put_str(FILE *f, const char *str)
{
    int res = 0;
    if(ISSET(f->status, _FST_NOBUF)) {
        if(f->writefn) {
            res = f->writefn(f->cookie, str, strlen(str));
        } else if(f->fd!=-1) {
            res = write(f->fd, str, strlen(str));
        } else
            return EOF;
        if(res <= 0)
            return EOF;
        return res;
    }
    bool lineBuf = ISSET(f->status, _FST_LINEBUF);
    while(*str) {
        char c = *(str++);
        f->buf[f->inbuf++] = c;
        if(f->inbuf == f->buf_size || (c == '\n' && lineBuf))
            fflush(f);
        res++;
    }
    return res;
}

static int min(int a, int b);
int
min(int a, int b)
{
    if(a<b) return a;
    return b;
}

int
__put_nstr(FILE *f, const char *str, int maxlen)
{
    int res = 0;
    if(ISSET(f->status, _FST_NOBUF)) {
        if(f->writefn) {
            res = f->writefn(f->cookie, str, min(strlen(str), maxlen));
        } else if(f->fd!=-1) {
            res = write(f->fd, str, min(strlen(str), maxlen));
        } else
            return EOF;
        if(res <= 0)
            return EOF;
        return res;
    }
    bool lineBuf = ISSET(f->status, _FST_LINEBUF);
    while(*str && maxlen-- > 0) {
        char c = *(str++);
        f->buf[f->inbuf++] = c;
        if(f->inbuf == f->buf_size || (c == '\n' && lineBuf))
            fflush(f);
        res++;
    }
    return res;
}

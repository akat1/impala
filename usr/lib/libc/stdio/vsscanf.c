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
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

static int _parse_int(const char **src, int base);
static void _parse_str(const char **src, char *res);
//static char digits[] = "0123456789abcdef";


#define todigit(c) (( (c) <= '9' )? (c) - '0' : (c) - 'a')
void
_parse_str(const char **src, char *res)
{
    while(!isspace(**src)) {
        *res = **src;
        (*src)++;
        res++;
    }
    *res = '\0';
}



int
_parse_int(const char **src, int base)
{
    int res=0;
    
    while(isdigit(**src)) {
        res = base*res + todigit(**src);
        (*src)++;
    }
    return res;
}

int
vsscanf(const char *src, const char *fmt, va_list ap)
{
    int res=0;
    const char *f = fmt;
    const char *s = src;
    while(*f) {
        if(*s == '\0')
            break;
        else if(*f == '%') {//jazda!
            f++;
            switch(*f) {
                case 's': {
                    char *sptr = va_arg(ap, char*);
                    _parse_str(&s, sptr);
                    res++;
                    break;
                }
                case 'd': {
                    int *iptr = va_arg(ap, int*);
                    *iptr = _parse_int(&s, 10);
                    res++;
                    break;
                }
                case 'o': {
                    int *iptr = va_arg(ap, int*);
                    *iptr = _parse_int(&s, 8);
                    res++;
                    break;
                }
                case 'c': {
                    char *cptr = va_arg(ap, char*);
                    *cptr = *s;
                    res++;
                    f++;
                    s++;
                    break;
                }
            }
        } else if(*f == *s) {
            //ok, zmatchowało się
            f++;
            s++;
            continue;
        } else {
            if(!res)
                res = EOF;
            break;
        }
    }
    return res;
}

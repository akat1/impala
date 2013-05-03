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
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>


int
snprintf(char *dst, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int res = vsnprintf(dst, size, fmt, ap);
    va_end(ap);
    return res;
}

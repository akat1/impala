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
#include <string.h>
#include <stdarg.h>

int
printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = vfprintf(stdout, format, ap);
    va_end(ap);
    return res;
}

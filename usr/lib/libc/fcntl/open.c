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
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

#include "libc_syscall.h"


int
open(const char *fname, int flags, ...)
{
    mode_t mode=0;
    va_list va;
    va_start(va, flags);
    if(flags & O_CREAT)
        mode = va_arg(va, mode_t);
    va_end(va);
    return syscall(SYS_open, fname, flags, mode);
}

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
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

#include "libc_syscall.h"


int
fcntl(int fd, int cmd, ...)
{
    int arg = 0;
    va_list va;
    va_start(va, cmd);
    arg = va_arg(va, int);
    va_end(va);
    return syscall(SYS_fcntl, fd, cmd, arg);
}

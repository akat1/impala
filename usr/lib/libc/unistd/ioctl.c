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
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdarg.h>

#include "libc_syscall.h"


int
ioctl(int fd, int cmd, ...)
{
    va_list va;
    va_start(va, cmd);
    uintptr_t param = 0;
    param = va_arg(va, uintptr_t); //no trudno...
    va_end(va);
    return syscall(SYS_ioctl, fd, cmd, param);
}

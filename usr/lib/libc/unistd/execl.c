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
#include <unistd.h>
#include <stdarg.h>

#include "libc_syscall.h"

int
execl(const char *path, const char *arg, ...)
{
    const int MAX_ARGS = 128;
    char *argv[MAX_ARGS];
    va_list va;
    va_start(va, arg);
    for(int i=0; i<MAX_ARGS; i++) {
        argv[i] = va_arg(va, char*);
        if(!argv[i])
            break;
    }
    va_end(va);
    return execve(path, argv, environ); //environ przekazujemy?
}



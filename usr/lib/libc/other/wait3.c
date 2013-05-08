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
#include <sys/wait.h>
#include <sys/syscall.h>
#include <errno.h>

pid_t
wait3(int *status, int options, struct rusage *rusage)
{
    int res = syscall(SYS_waitpid, -1, status, options);
    if(errno == ECHILD && options & WNOHANG)
        return 0;
    return res;
}

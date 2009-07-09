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
#include <sys/wait.h>
#include <sys/syscall.h>
#include <errno.h>

pid_t
waitpid(pid_t pid, int *status, int options)
{
    int res = syscall(SYS_waitpid, pid, status, options);
    if(errno == ECHILD && options & WNOHANG)
        return 0;
    return res;
}

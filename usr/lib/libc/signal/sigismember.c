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
#include <sys/types.h>
#include <sys/signal.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigismember(const sigset_t *set, int signum)
{
    if ( signum < 1 || signum > _NSIG )
        return -1;
    
    if ( *set & sigmask(signum) )
        return 1;
    else
        return 0;
}

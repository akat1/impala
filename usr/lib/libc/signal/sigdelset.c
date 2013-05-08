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
#include <sys/types.h>
#include <sys/signal.h>
#include <signal.h>

#include "libc_syscall.h"

int
sigdelset(sigset_t *set, int signum)
{
    if ( signum < 1 || signum > _NSIG )
        return -1;
    
    *set &= ~sigmask(signum);
    return 0;
}

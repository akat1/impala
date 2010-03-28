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
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "libc_syscall.h"

int
sigaction(int signum, const sigaction_t *act, sigaction_t *oldact)
{
    sighandler_t handler;
    sigaction_t myact;
    int r; 

    if ( signum < 0 && signum > NSIG ) {
        errno = EINVAL;
        return -1;
    }

    if ( act != NULL ) {
        memcpy(&myact, act, sizeof(sigaction_t));
        handler = act->sa_handler;
        r = syscall(SYS_sigaction, signum, myact, oldact);
    } else {
        r = syscall(SYS_sigaction, signum, act, oldact);
    }


    if ( !r ) {
        if ( act != NULL ) {
            __sig_handlers[signum] = handler;
        }
        return 0;
    } else {
        return -1;
    }
}

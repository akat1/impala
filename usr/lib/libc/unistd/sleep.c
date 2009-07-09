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
#include <time.h>

#include <unistd.h>

#include "libc_syscall.h"

unsigned int
sleep(unsigned int secs)
{
    timespec_t req;
    timespec_t el;
    req.tv_sec = secs;
    req.tv_nsec = 0;
    if (nanosleep(&req, &el)) return -1;
    return el.tv_sec;
}

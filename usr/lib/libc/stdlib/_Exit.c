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
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdlib.h>

void
_Exit(int status)
{
    syscall(SYS_exit, status);
}

//mam nadzieję, że nie zostanę pobity za dwie funkcje w 1 pliku... ;p
void
_exit(int status)
{
    syscall(SYS_exit, status);
}

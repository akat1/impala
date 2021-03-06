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
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio_private.h>

__noreturn void
exit(int status)
{
    FILE *f = NULL;
    while ((f = list_head(&__open_files)))
        fclose(f);
    syscall(SYS_exit, status);
}

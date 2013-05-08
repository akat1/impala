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
#include <stdio.h>
#include <string.h>
#include <errno.h>
//todo: lista błędów - ich opisów

void
perror(const char *s)
{
    if(!s)
        fprintf(stderr, "%s\n", strerror(errno));
    else
        fprintf(stderr, "%s: %s\n",s, strerror(errno));
}

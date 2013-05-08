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
#ifndef __FNMATCH_H__
#define __FNMATCH_H__

int fnmatch(const char *pattern, const char *string, int flags);

#define NM_NOMATCH      (1<<0)
#define FNM_PATHNAME    (1<<1)
#define FNM_PERIOD      (1<<2)
#define FNM_NOESCAPE    (1<<3)
#define FNM_NOSYS       (1<<4)


#endif

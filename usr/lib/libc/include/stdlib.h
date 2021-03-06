/* Impala C Library
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */
#ifndef __STDLIB_H
#define __STDLIB_H

#include <sys/types.h>
#include <stdio.h>

void    _exit(int status);
void    _Exit(int status);
__noreturn void    abort(void);
int     atoi(const char *nptr);
long    atol(const char *nptr);
void   *calloc(size_t nmemb, size_t size);
__noreturn void    exit(int status);
void    free(void *ptr);
char   *getenv(const char *name);
void   *malloc(size_t size);
void   *realloc(void *ptr, size_t size);
int     setenv(const char *name, const char *value, int overwrite);
int     unsetenv(const char *name);
long int strtol(const char *nptr, char **endptr, int base);
int     system(const char *command);
void    qsort(void *base, size_t nmemb, size_t size,
                int(*compar)(const void *, const void *));
void   *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
                int(*compar)(const void *, const void *));
long int strotl(const char *nptr, char **endptr, int base);
unsigned long int strtoul(const char *nptr, char **endptr, int base);

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define ALIGN(x) x // XXX: I've no idea what's this


#endif

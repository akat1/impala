/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
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
#ifndef __STRING_H
#define __STRING_H

#include <sys/types.h>

void  *memchr(const void *s, int c, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);
void  *memcpy(void *dest, const void *src, size_t n);
void  *memmove(void *dest, const void *src, size_t n);
void  *memset(void *s, int c, size_t n);
char  *strcat(char *dest, const char *src);
char  *strchr(const char *s, int c);
int    strcmp(const char *s1, const char *s2);
int    strcoll(const char *s1, const char *s2);
char  *strcpy(char *dest, const char *src); 
size_t strcspn(const char *s, const char *reject);
char  *strerror(int); //
size_t strlen(const char *);
char  *strncat(char *dest, const char *src, size_t n);
int    strncmp(const char *s1, const char *s2, size_t n);
char  *strncpy(char *dest, const char *src, size_t n); 
char  *strpbrk(const char *s, const char *accept);
char  *strrchr(const char *s, int c);
size_t strspn(const char *s, const char *accept);
char  *strstr(const char *haystack, const char *needle); //
char  *strtok(char *s, const char *delim); 
size_t strxfrm(char *dest, const char *src, size_t n);

//strings.h bardziej... no ale:
char *rindex(const char *s, int c);
int   strcasecmp(const char *s1, const char *s2);
int   strncasecmp(const char *s1, const char *s2, size_t n);



#endif



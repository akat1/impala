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

#ifndef __SYS_STRING_H
#define __SYS_STRING_H

#ifdef __KERNEL

addr_t mem_move(addr_t dst, const addr_t src, size_t len);
addr_t mem_cpy(addr_t dst, const addr_t src, size_t len);
addr_t mem_set(addr_t s, char c, size_t len);
addr_t mem_set16(addr_t s, uint16_t c, size_t len);
size_t str_len(const char *s);
int str_cmp(const char *a, const char *b);
char * str_cat(char *str, const char *s);
char * str_cpy(char *str, const char *s);
char * str_ncpy(char *str, const char *s, size_t len);
char * str_dup(const char *s);

int snprintf(char *dst, size_t size, const char *fmt, ...);
int vsnprintf(char *dst, size_t size, const char *fmt, va_list ap);

enum {
    SPRINTF_BUFSIZE = 512
};


#define mem_zero(s, l) mem_set(s, 0, l)
#define str_eq(a,b) (str_cmp(a,b) == 0)
#endif

#endif

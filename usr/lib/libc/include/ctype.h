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

#ifndef __CTYPE_H
#define __CTYPE_H

//is in range
#define __iir(c, a, b) ((a<=(c)) && ((c)<=b))

#define isalnum(c) (isalpha(c) || isdigit(c))
#define isalpha(c) (isupper(c) || islower(c))
#define isascii(c) (!((c)&(~0x7f)))
#define isblank(c) ((c)==' ' || (c)=='\t')
#define iscntrl(c) ((c) == 0177 || __iir((c), 0, ' '-1))
#define isdigit(c) __iir((c), '0', '9')
///@todo do punct sporo różnych należy... może warto by było używać locale?
#define ispunct(c) (__iir((c), '!', '/') || __iir((c), ':', '@') \
                  || __iir((c), '[', '`') || __iir((c), '{', '~'))
#define isgraph(c) (isalnum(c) || ispunct(c))
#define islower(c) __iir((c), 'a', 'z')
#define isprint(c) (__iir((c), ' ', 126))

#define isspace(c) ((c)==' ' || (c)=='\f' || (c)=='\n' || (c)=='\r' \
                    || (c)=='\t' || (c)=='\v')
#define isupper(c) __iir((c), 'A', 'Z')
#define isxdigit(c) (isdigit(c) || __iir((c), 'a', 'f') || __iir((c), 'A', 'F'))
#define tolower(c) ((c) >= 'A'&& (c) <= 'Z'?(c-('A'-'a')):(c))
#define toupper(c) ((c) >= 'a'&& (c) <= 'z'?(c-('a'-'A')):(c))


#endif

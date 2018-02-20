/*
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

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/kargs.h>

extern char kernarg[256];

typedef struct karg karg_t;
struct karg {
    const char *name;
    const char *value;
};

static int parse_int(const char *s);

static karg_t kargs[20];
static int kargc;
static char *name = 0;

void
kargs_init()
{
    int needv;
    char *kargv = kernarg;
    name = kargv;
    while (*kargv && *kargv != ' ') kargv++;
    if (*kargv == ' ') {
        *kargv = 0;
        kargv++;
    }

    for (kargc = 0; kargc < 20 && *kargv; kargc++) {
        needv = 0;
        kargs[kargc].name = kargv;
        kargs[kargc].value = "";
        while (*kargv && *kargv != ' ') {
            if (needv) {
                kargs[kargc].value = kargv;
                needv = 0;
            }
            if (*kargv == '=') {
                *kargv = 0;
                needv = 1;
            }
            kargv++;
        }
        if (*kargv == ' ') *kargv = 0;
        kargv++;
    }
}

bool
karg_is_set(const char *name)
{
    for (int i = 0; i < kargc; i++) {
        if (strcmp(name, kargs[i].name) == 0) return TRUE;
    }
    return FALSE;
}

int
karg_get_s(const char *name, const char **s)
{
    for (int i = 0; i < kargc; i++) {
        if (strcmp(name, kargs[i].name) == 0) {
            *s = kargs[i].value;
            return 0;
        }
    }
    return -1;
}

const char *
karg_get_name()
{
    return name;
}

int
karg_get_i(const char *name, int *x)
{
    for (int i = 0; i < kargc; i++) {
        if (strcmp(name, kargs[i].name) == 0) {
            *x = parse_int(kargs[i].value);
            return 0;
        }
    }
    return -1;
}

int
parse_int(const char *s)
{
    int base = 10;
    int i = 0;
    char sign = 1;
    if (s[0] == '-') {
        sign = -1; s++;
    }
    if (s[0] == '0' && s[1] == 'x') {
        s += 2;
        base = 16;
    }
    while (*s) {
        i *= base;
        int digit = *s - '0';
        if (digit >= 10) digit = 10 + *s - 'a';
        i += digit;
        s++;
    }
    return i*sign;
}

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

#include <sys/types.h>
#include <sys/string.h>
#include <sys/kprintf.h>
#include <machine/video.h>

#define DEFAULT_ATTRIBUTE (COLOR_WHITE)



enum {
    KPRINTF_BUF = 2048
};

void
kprintf(const char *fmt, ...)
{
    va_list ap;
    VA_START(ap, fmt);
    vkprintf(fmt, ap);
    VA_END(ap);
}

void
vkprintf(const char *fmt, va_list ap)
{
    char big_buf[KPRINTF_BUF];
    char *ptr=big_buf;
    mem_zero(big_buf, KPRINTF_BUF);
    vsnprintf(big_buf, KPRINTF_BUF, fmt, ap);
    
    for (; *ptr; ptr++) {
        switch (*ptr) {
            case '\n':
                textscreen_next_line(NULL);
                break;
            case '\t':
                for(int j=0; j<6; j++)
                    textscreen_put(NULL, ' ', DEFAULT_ATTRIBUTE);
                break;

            default:
                textscreen_put(NULL, *ptr, DEFAULT_ATTRIBUTE);
                break;
        }
    }

//   textscreen_draw(NULL);
}

/* Impala Operating System
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

#ifndef __SYS_UTILS_H
#define __SYS_UTILS_H

#ifdef __KERNEL
void panic(const char* msg, ...);
extern bool SYSTEM_DEBUG;

#define KASSERT(x) if(!(x)) \
    panic("Assertion failed\n expr: %s\n in file: %s:%u\n in function: %s",\
        #x , __FILE__, __LINE__,  __func__);


static inline int min(int a, int b)
{
    if(a<b) return a;
    return b;
}

static inline int max(int a, int b)
{
    if(a<b) return b;
    return a;
}
        
#define MIN(a,b) (( (a) < (b) )? (a) : (b))
#define MAX(a,b) (( (a) < (b) )? (b) : (a))
#define _INRNG(x,a,b,R1,R2) ((a) R1 (x)) && ((x) R2 (b))
#define _INRNG_L(x,a,l,R1,R2) _INRNG(x,a,a+l,R1,R2)
#define INRNG_OOL(x,a,l) _INRNG_L(x,a,l,<,<)
#define INRNG_OCL(x,a,l) _INRNG_L(x,a,l,<,<=)
#define INRNG_COL(x,a,l) _INRNG_L(x,a,l,<=,<)
#define INRNG_CCL(x,a,l) _INRNG_L(x,a,l,<=,<=)

void iprintf(const char *fmt, ...);
void kprintf(const char *fmt, ...);
void vkprintf(const char *fmt, va_list ap);

ssize_t copyin(void *kaddr, const void *uaddr, size_t len);
ssize_t copyout(void *uaddr, const void *kaddr, size_t len);

ssize_t copyinstr(void *kaddr, const void *uaddr, size_t limit);
ssize_t copyoutstr(void *uaddr, const void *kaddr, size_t limit);

#define TRACE_IN(fmt, args...)\
    do { \
        if (!SYSTEM_DEBUG) break;\
        kprintf("%s(", __func__);\
        kprintf(fmt, ## args);\
        kprintf(")\n");\
        for (unsigned int xxx = 0; xxx < 0xfffff; xxx++);\
    } while (0);

#define DEBUGF(fmt, a...) do {\
    if (!SYSTEM_DEBUG) break;\
    kprintf("%s.%03u: " fmt "\n", __LS(__FILE__), __LINE__, ## a );\
    } while (0)

#define IDEBUGF(fmt, a...) do {\
    if (!SYSTEM_DEBUG) break;\
    iprintf("%s.%03u: " fmt "\n", __LS(__FILE__), __LINE__, ## a );\
    } while (0)

#define TRACE_IN0() TRACE_IN("");

static inline const char *
__LS(const char *file)
{
    const char *last;
    for (last = file; *file; file++) {
        if (*file == '/') last = file+1;
    }
    return last;
}

#else   //not in kernel

int printf(const char *, ...);
#define KASSERT(x) if(!(x)){ \
    printf("Assertion failed\n expr: %s\n inile: %s:%u\n in function: %s",\
        #x , __FILE__, __LINE__,  __func__);while(1);};

#endif
#endif


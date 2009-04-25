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

#ifndef __MACHINE_IO_H
#define __MACHINE_IO_H

#include <sys/types.h>

// Lepiej w funkcjach ni¿ makrach - kompilator sobie sprawdzi typy,
// a przy -O1 i tak powinien to zoptymalizowaæ do jednej instrukcji.

static inline uint32_t
io_in32(uint16_t port)
{
    uint val;
    __asm__ volatile("inl %%dx, %%eax":"=a" (val):"d" (port));
    return val;
}

static inline void
io_out32(uint16_t port, uint32_t val)
{
    __asm__ volatile("outl %%eax, %%dx"::"a" (val), "d" (port));
}

static inline uint16_t
io_in16(uint16_t port)
{
    uint16_t val;
    __asm__ volatile("inw %%dx, %%ax":"=a" (val):"d" (port));
    return val;
}

static inline void
io_out16(ushort port, uint16_t val)
{
    __asm__ volatile("outw %%ax, %%dx"::"a" (val), "d" (port));
}

static inline uint8_t
io_in8(uint16_t port)
{
    uint8_t val;
    __asm__ volatile("inb %%dx, %%al":"=a" (val):"d" (port));
    return val;
}

static inline void
io_out8(ushort port, uint8_t val)
{
    __asm__ volatile("outb %%al, %%dx"::"a" (val), "d" (port));
}

#endif



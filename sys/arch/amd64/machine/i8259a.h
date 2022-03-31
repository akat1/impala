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

#ifndef __MACHINE_i8259A_H
#define __MACHINE_i8259A_H

#include <machine/interrupt.h>

enum {
    PIC_IRQ0 = 0x01,
    PIC_IRQ1 = 0x02,
    PIC_IRQ2 = 0x04,
    PIC_IRQ3 = 0x08,
    PIC_IRQ4 = 0x10,
    PIC_IRQ5 = 0x20,
    PIC_IRQ6 = 0x40,
    PIC_IRQ7 = 0x80
};

enum {
    PIC_IRQ8 = 0x01,
    PIC_IRQ9 = 0x02,
    PIC_IRQ10 = 0x04,
    PIC_IRQ11 = 0x08,
    PIC_IRQ12 = 0x10,
    PIC_IRQ13 = 0x20,
    PIC_IRQ14 = 0x40,
    PIC_IRQ15 = 0x80
};

#ifdef __KERNEL

void i8259a_init(void);
void i8259a_send_eoi(void);
void i8259a_reset_mask(void);
void i8259a_irq_enable(int n);
void i8259a_irq_disable(int n);
void i8259a_set_irq_priority(int n, int ipl);

extern int irq_priority[MAX_IRQ];

#endif

#endif

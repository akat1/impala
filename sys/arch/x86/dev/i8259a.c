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
#include <machine/i8259a.h>
#include <machine/io.h>
#include <machine/interrupt.h>

enum {
    PIC_M = 0x20,
    PIC_S = 0xA0
};

enum {
    ICW1_ICW4       = 0x01,
    ICW1_CASCADE    = 0x02,
    ICW1_INTERVAL8  = 0x00,
    ICW1_INTERVAL4  = 0x04,
    ICW1_TRIG_EDGE  = 0x00,
    ICW1_TRIG_LEVEL = 0x08,
    ICW1_RESET      = 0x10
};


enum {
    ICW4_8086          = 0x01,
    ICW4_EOI_AUTO      = 0x02,
    ICW4_EOI_NORMAL    = 0x00,
    ICW4_BUF_NONE      = 0x00,
    ICW4_BUF_MASTER    = 0x00,
    ICW4_BUF_SLAVE     = 0x00,
    ICW4_NESTED        = 0x80
};

enum {
    OCW2_EOI_NORMAL = 0x20
};

static uchar pic1_mask;
static uchar pic2_mask;


void
i8259a_init()
{
    // ICW1
    io_out8(PIC_M, ICW1_RESET | ICW1_ICW4);
    // ICW2
    io_out8(PIC_M+1, INTERRUPT_VECTOR);   
    // ICW3
    io_out8(PIC_M+1, 0x04);
    // ICW4
    io_out8(PIC_M+1, ICW4_8086);

    // ICW1
    io_out8(PIC_S, ICW1_RESET | ICW1_ICW4);
    // ICW2
    io_out8(PIC_S+1, INTERRUPT_VECTOR+0x8);   
    // ICW3
    io_out8(PIC_S+1, 0x02);
    // ICW4
    io_out8(PIC_S+1, ICW4_8086);

    pic1_mask = 0x0;
    pic2_mask = 0x0;
    i8259a_reset_mask();
}


void
i8259a_reset_mask()
{
    io_out8(PIC_M+1, ~pic1_mask);
    io_out8(PIC_S+1, ~pic2_mask);
}

void
i8259a_irq_enable(int n)
{
    if (n < 0x8)
        pic1_mask |= 1 << n;
    else
        pic2_mask |= 1 << (n-8);
    i8259a_reset_mask();
}

void
i8259a_irq_disable(int n)
{
    if (n < 0x8)
        pic1_mask &= ~(1 << n);
    else
        pic2_mask &= ~(1 << (n-8));
    i8259a_reset_mask();
}

void
i8259a_send_eoi()
{
    io_out8(PIC_M, OCW2_EOI_NORMAL);
    io_out8(PIC_S, OCW2_EOI_NORMAL);
}


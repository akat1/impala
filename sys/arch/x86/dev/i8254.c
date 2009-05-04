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
#include <sys/clock.h>
#include <sys/kprintf.h>
#include <sys/utils.h>
#include <machine/interrupt.h>
#include <machine/i8254.h>
#include <machine/i8259a.h>
#include <machine/io.h>

static bool i8254_irq0(void);


void
i8254_init()
{
    i8254_set_freq(HZ);
    irq_install_handler(IRQ0, i8254_irq0, IPL_CLOCK);
}

bool
i8254_irq0()
{
    clock_hardtick();
    irq_done();
    clock_softtick();
    return TRUE;
}

/**
 * Ustawia czêstotliwo¶æ z jak± PIT generuje IRQ 0
 */

void
i8254_set_freq(uint hz)
{
    KASSERT(hz >= 19);  // wymagane, aby wynik mie¶ci³ siê w 2 bajtach
    
    uint res=PIT_MAX_FREQ/hz;
    
    io_out8(PIT_MODE, 0x34); // licznik zero w trybie 2,
                             // przesy³ane oba bajty kodowanej binarnie liczby
    io_out8(PIT_CHAN0, res&0xff);
    io_out8(PIT_CHAN0, (res>>8)&0xff);
}


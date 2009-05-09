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
#include <sys/utils.h>
#include <sys/string.h>
#include <sys/console.h>
#include <machine/interrupt.h>

/**
 * Funkcja wywo³ywana w sytuacjach awaryjnych.
 * Zatrzymuje system, wy¶wietlaj±c podany komunikat.
 */

bool SYSTEM_DEBUG = FALSE;

void
panic(const char *msg, ...)
{
    irq_disable();
    va_list ap;
    VA_START(ap, msg);
    kprintf("\npanic: ");
    vkprintf(msg, ap);
    kprintf("\n");
    VA_END(ap);
    while(1);
}


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
    enum {
        KPRINTF_BUF = 512
    };
    char big_buf[KPRINTF_BUF];
    vsnprintf(big_buf, KPRINTF_BUF, fmt, ap);
    cons_msg(big_buf);
}


int
splhigh()
{
    irq_disable();
    TRACE_IN("enter");
    int opl=intrpt_getipl();
    intrpt_raiseipl(IPL_HIGH);
    irq_enable();
    TRACE_IN("leave");
    return opl;
}

int
splclock()
{
    irq_disable();
    int opl=intrpt_getipl();
    if(opl < IPL_CLOCK)
        intrpt_raiseipl(IPL_CLOCK);
    irq_enable();
    return opl;
}

int
splbio()
{
    irq_disable();
    int opl=intrpt_getipl();
    if(opl < IPL_BIO)
        intrpt_raiseipl(IPL_BIO);
    irq_enable();
    return opl;
}

int
spltty()
{
    irq_disable();
    int opl=intrpt_getipl();
    if(opl < IPL_TTY)
        intrpt_raiseipl(IPL_TTY);
    irq_enable();
    return opl;
}

int
spl0()
{
    irq_disable();
    int opl=intrpt_getipl();
    intrpt_loweripl(IPL_NONE);
    irq_enable();
    return opl;
}

void
splx(int pl)
{
    irq_disable();
    int opl=intrpt_getipl();
    if(opl > pl)
        intrpt_loweripl(pl);
    else
        intrpt_raiseipl(pl);
    irq_enable();
}



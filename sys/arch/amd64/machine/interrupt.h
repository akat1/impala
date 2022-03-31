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

#ifndef __MACHINE_INTERRUPT_H
#define __MACHINE_INTERRUPT_H


enum {
    INTERRUPT_VECTOR = 0x20,
    INTRPT_SCHED = 0x70,
    INTRPT_SYSCALL = 0x80
};

enum {
    IRQ0 = 0,
    IRQ1,
    IRQ2,
    IRQ3,
    IRQ4,
    IRQ5,
    IRQ6,
    IRQ7,
    IRQ8,
    IRQ9,
    IRQ10,
    IRQ11,
    IRQ12,
    IRQ13,
    IRQ14,
    IRQ15,
    MAX_IRQ
};

enum {
    IPL_NONE=0,
    IPL_0=0,
    IPL_TTY=1,
    IPL_NET=2,
    IPL_BIO=3,
    IPL_SOFTCLOCK=3,
    IPL_CLOCK=4,
    IPL_HIGH=5,
    MAX_IPL
};

typedef bool irq_handler_f(void);

typedef struct interrupt_frame interrupt_frame;
struct interrupt_frame {
    // pusha
    uint64_t test;
    uint64_t f_rdi;
    uint64_t f_rsi;
    uint64_t f_rbp;
    uint64_t f_isp;
    uint64_t f_rbx;
    uint64_t f_rdx;
    uint64_t f_rcx;
    uint64_t f_rax;

    // recznie
    uint64_t f_gs;
    uint64_t f_fs;
    uint64_t f_es;
    uint64_t f_ds;

    uint64_t f_n;
    uint64_t f_errno;
    uint64_t f_rip;
    uint64_t f_cs;
    uint64_t f_rflags;

    uint64_t f_rsp;
    uint64_t f_ss;
} __packed;


#ifdef __KERNEL
void irq_install_handler(int irq, irq_handler_f *h, int ipl);
void irq_free_handler(int irq);

extern int volatile CIPL;
extern bool wantSched;


/// Wyłącza obsługę przerwań
static inline void irq_disable(void)
{
    __asm__ volatile("cli");
}

/// Włącza obsługę przerwań przez procesor
static inline void irq_enable(void)
{
    __asm__ volatile("sti");
}


int splhigh(void);
int splclock(void);
int splbio(void);
#define splsoftclock splbio
int spltty(void);
int spl0(void);
void splx(int pl);

#endif


#endif

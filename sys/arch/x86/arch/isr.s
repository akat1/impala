# Impala Operating System
#
# Copyright (C) 2009 University of Wroclaw. Department of Computer Science
#    http://www.ii.uni.wroc.pl/
# Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
#    http://bitbucket.org/wieczyk/impala/
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#  $Id$
#

#
# isr.s (interrupt service routine)
#
# Plik zawiera niskopoziomowe procedury i uchwyty do obslugi przerwañ.
#

.text
.global _intrpt_syscall

.set text_selector, 0x08
.set data_selector, 0x10


.macro PUSH_FRAME
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal
    push $0xdeadbab2
.endm

.macro SET_KERNEL_REGS
    movl $data_selector, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs
.endm

.macro POP_FRAME
    pop %eax
    popal
    pop %gs
    pop %fs
    pop %es
    pop %ds
.endm

.macro TRAP num, name, handler
\name:
    cli
#    pop %esi
#    pop %esi
#    jmp .
    push $\num
    PUSH_FRAME
    SET_KERNEL_REGS
    call \handler
    POP_FRAME
    add $4, %esp
    sti
    iret
.endm

.macro TRAPNE num, name, handler
\name:
    cli
    push $0
    push $\num
    PUSH_FRAME
    SET_KERNEL_REGS
    call \handler
    POP_FRAME
    add $8, %esp
    sti
    iret
.endm


.macro ISR num, name, handler
\name:
    cli
    push $\num
    PUSH_FRAME
    SET_KERNEL_REGS
    call \handler
    POP_FRAME
    add $4, %esp
    sti
    iret
.endm

.global _unhnd_intrpt


TRAP 0, _unhnd_intrpt, TRAP_unhandled

TRAP 0, _trap0, TRAP_unhandled
TRAP 1, _trap1, TRAP_unhandled
TRAP 2, _trap2, TRAP_unhandled
TRAP 3, _trap3, TRAP_unhandled
TRAP 4, _trap4, TRAP_unhandled
TRAP 5, _trap5, TRAP_unhandled
TRAP 6, _trap6, TRAP_unhandled
TRAP 7, _trap7, TRAP_unhandled
TRAP 8, _trap8, TRAP_unhandled
TRAP 9, _trap9, TRAP_unhandled
TRAP 10, _trap10, TRAP_unhandled
TRAP 11, _trap11, TRAP_unhandled
TRAP 12, _trap12, TRAP_unhandled
TRAP 13, _trap13, TRAP_gfault
TRAP 14, _trap14, TRAP_pfault
TRAP 15, _trap15, TRAP_unhandled
TRAP 16, _trap16, TRAP_unhandled

TRAPNE 80, _intrpt_syscall, ISR_syscall

ISR 0x00, _isr0, ISR_irq
ISR 0x01, _isr1, ISR_irq
ISR 0x02, _isr2, ISR_irq
ISR 0x03, _isr3, ISR_irq
ISR 0x04, _isr4, ISR_irq
ISR 0x05, _isr5, ISR_irq
ISR 0x06, _isr6, ISR_irq
ISR 0x07, _isr7, ISR_irq
ISR 0x08, _isr8, ISR_irq
ISR 0x09, _isr9, ISR_irq
ISR 0x0a, _isr10, ISR_irq
ISR 0x0b, _isr11, ISR_irq
ISR 0x0c, _isr12, ISR_irq
ISR 0x0d, _isr13, ISR_irq
ISR 0x0e, _isr14, ISR_irq
ISR 0x0f, _isr15, ISR_irq
ISR 0x10, _isr16, ISR_irq
ISR 0x11, _isr17, ISR_irq
ISR 0x12, _isr18, ISR_irq
ISR 0x13, _isr19, ISR_irq
ISR 0x14, _isr20, ISR_irq
ISR 0x15, _isr21, ISR_irq
ISR 0x16, _isr22, ISR_irq
ISR 0x17, _isr23, ISR_irq


.data
.global irq_table
.global trap_table

#
# Tablice wskaznikow uchwytow do przerwan i pulapek, dzieki nim
# nie trzeba eksportowac symboli do nich.
#

irq_table:
    .long _isr0
    .long _isr1
    .long _isr2
    .long _isr3
    .long _isr4
    .long _isr5
    .long _isr6
    .long _isr7
    .long _isr8
    .long _isr9
    .long _isr10
    .long _isr11
    .long _isr12
    .long _isr13
    .long _isr14
    .long _isr15
    .long _isr16
    .long _isr17
    .long _isr18
    .long _isr19
    .long _isr20
    .long _isr21
    .long _isr22
    .long _isr23
    .long 0

trap_table:
    .long _trap0
    .long _trap1
    .long _trap2
    .long _trap3
    .long _trap4
    .long _trap5
    .long _trap6
    .long _trap7
    .long _trap8
    .long _trap9
    .long _trap10
    .long _trap11
    .long _trap12
    .long _trap13
    .long _trap14
    .long _trap15
    .long _trap16
    .long 0


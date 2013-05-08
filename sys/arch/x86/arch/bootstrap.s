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

.global kernel_entrypoint
.global boot_ptable
.global boot_pdir
.global KERNEL_START
.global KERNEL_BOOTSTRAP
.global boot_ptable0
.global boot_ptable1
.global boot_pdir
#
# Sekcja .bootstrap jest sekcja rozruchowa jadra.
# Jest ladowana pod adres KERNEL_BOOTSTRAP. Konsolidator
# ustawia rowniez taki jej adres wirtualny.
# Kod ma za zadanie przygotowac sprzet do uruchomienia jadra wlasciwego
# (znajdujacego sie pod adresami KERNEL_START).


.section .bootstrap

.equ KERNEL_BOOTSTRAP, 0x100000
.equ KERNEL_START, 0xc0000000

.equ ALIGN,    1<<0
.equ MEMINFO,  1<<1
.equ CMDLINE,  1<<2
.equ ADDRESS,  1<<16
.equ FLAGS,    ALIGN|CMDLINE
.equ MAGIC,    0x1BADB002
.equ CHECKSUM, -(MAGIC + FLAGS)

.equ PTE_PRESENT,   1<<0
.equ PTE_RW,        1<<1
.equ PTE_US,        1<<2
.equ PTE_G,         1<<8
.equ PTE_ATTR,      (PTE_PRESENT|PTE_RW|PTE_US)
.equ PAGE_SIZE,     4096
.equ CR0_PG,        1<<31



# naglowek MULTIBOOT dla programu GRUB.

#Offset 	Type 	Field Name 	Note

multiboot_header:
.align 4
#0 	u32 	magic 	required
.long MAGIC
#4 	u32 	flags 	required
.long FLAGS
#8 	u32 	checksum 	required
.long CHECKSUM
#12 	u32 	header_addr 	if flags[16] is set
.long 0
#16 	u32 	load_addr 	if flags[16] is set
.long 0
#20 	u32 	load_end_addr 	if flags[16] is set
.long 0
#24 	u32 	bss_end_addr 	if flags[16] is set
.long 0
#28 	u32 	entry_addr 	if flags[16] is set
.long 0
#32 	u32 	mode_type 	if flags[2] is set
.long 1
#36 	u32 	width 	if flags[2] is set
.long 80
#40 	u32 	height 	if flags[2] is set
.long 25
#44 	u32 	depth 	if flags[2] is set 
.long 0

cmdline:    .long 0

.align 4096
boot_pdir: .space 4096,0
boot_ptable0: .space 4096,0
boot_ptable1: .space 4096,0

.set STACKSIZE, 256
.comm stack, STACKSIZE, 32
.set EFLAGS_ID, 1 << 21
kernel_entrypoint:
    movl $(stack + STACKSIZE-4), %esp
    movl 16(%ebx), %eax
    movl %eax, cmdline
    call check_cpuid
    call set_vmspace
    pushl cmdline
    call kernel_startup
.L1:
    movl $0xb8004, %edx
    incw (%edx)
    jmp .L1

# void check_cpuid(void)
check_cpuid:
    pushfl
    popl %eax
    pushl %eax
    andl $EFLAGS_ID, %eax
    popl %eax
    jnz ccp.1
    or $EFLAGS_ID, %eax
    pushl %eax
    popfl
    pushfl
    popl %eax
    andl $EFLAGS_ID, %eax
    jz ccp.3
    jmp ccp.2
ccp.1:    # sprawdzenie mo¿liwo¶ci czyszczenia bitu
    movl $EFLAGS_ID, %ebx
    notl %ebx
    andl %ebx, %eax
    pushl %eax
    popfl
    pushfl
    popl %eax
    andl $EFLAGS_ID, %eax
    jnz ccp.3
ccp.2:    # mo¿na modyfikowaæ bit ID
    ret
ccp.3:    # nie mo¿na modyfikowaæ bitu ID, to nie jest i586 nawet, zawieszamy siê
    jmp .


# void set_vmspave(void)
set_vmspace:
    movl $boot_ptable0, %ebx
    or $PTE_ATTR, %ebx
    movl %ebx, boot_pdir

    movl $boot_ptable1, %ebx
    or $PTE_ATTR, %ebx
    movl $KERNEL_START >> 20, %eax
    movl %ebx, boot_pdir(%eax)

# odwzorowuje pierwsze 4MB
    pushl $0
    pushl $boot_ptable0
    call fill_ptable
    addl $8, %esp

# odwzorowuje 4MB z kodem jadra
    pushl $KERNEL_BOOTSTRAP
    pushl $boot_ptable1
    call fill_ptable
    addl $8, %esp

# wlaczamy stronicowanie
    movl $boot_pdir, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    or $CR0_PG, %eax
    movl %eax, %cr0


    ret

tick:
    movl $0xb8000, %edx
    incw (%edx)
    ret

.global tick

# odwzorowuje 4MB pamieci w danej tablicy stron
# void fill_ptable(ptable, paddr)
# ptable 8 %edi
# paddr 12 %ebx
fill_ptable:
    enter $0, $0
    movl 8(%ebp), %edi
    movl 12(%ebp), %ebx
    movl $1024, %ecx
    or $PTE_ATTR, %ebx
.2:
    movl %ebx, (%edi)
    addl $PAGE_SIZE, %ebx
    addl $4, %edi
    loop .2
    leave
    ret




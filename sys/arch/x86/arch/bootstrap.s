# Impala Operating System
#
# Copyright (C) 2009 University of Wroclaw. Department of Computer Science
#    http://www.ii.uni.wroc.pl/
# Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
#    http://trzask.codepainters.com/impala/trac/
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
.global megaloop
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

multiboot_header:
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM


cmdline:    .long 0

.align 4096
boot_pdir: .space 4096,0
boot_ptable0: .space 4096,0
boot_ptable1: .space 4096,0

.set STACKSIZE, 256
.comm stack, STACKSIZE, 32

kernel_entrypoint:
    movl $(stack + STACKSIZE-4), %esp
    movl 16(%ebx), %eax
    movl %eax, cmdline
    call set_vmspace
    pushl cmdline
    call kernel_startup
.L1:
    movl $0xb8004, %edx
    incw (%edx)
    jmp .L1

# void set_vmspave()
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

megaloop:
    jmp megaloop

.asciz "|/-\\"


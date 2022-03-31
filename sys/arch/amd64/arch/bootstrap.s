/*
 * Copyright (C) 2022 Mateusz Kocielski
 *
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
 */

/*
 * Reading material:
 *
 * https://www.iaik.tugraz.at/teaching/materials/os/tutorials/paging-on-intel-x86-64/
 * https://www.moritz.systems/blog/before-the-bsd-kernel-starts-part-one-on-amd64/
 * https://en.wikipedia.org/wiki/Physical_Address_Extension
 *
 * amd64 implementation requires PAE enabled which uses 3 levels of tables:
 * 
 * PDPT -> PDT -> PT
 *
 * PDPT - page directory pointer table
 * PDT - page directory table
 * PT - page table
 */


.global kernel_entrypoint
.global boot_ptable
.global boot_pdir
.global KERNEL_START
.global KERNEL_BOOTSTRAP
.global boot_ptable0
.global boot_ptable1
.global boot_pdir


# Sekcja .bootstrap jest sekcja rozruchowa jadra.
# Jest ladowana pod adres KERNEL_BOOTSTRAP. Konsolidator
# ustawia rowniez taki jej adres wirtualny.
# Kod ma za zadanie przygotowac sprzet do uruchomienia jadra wlasciwego
# (znajdujacego sie pod adresami KERNEL_START).

.section .bootstrap

.equ KERNEL_BOOTSTRAP, 0x100000
.equ KERNEL_START, 0xffffffff80000000

.equ ALIGN,    1<<0
.equ MEMINFO,  1<<1
.equ CMDLINE,  1<<2
.equ ADDRESS,  1<<16
.equ FLAGS,    ALIGN|CMDLINE
.equ MAGIC,    0x1BADB002
.equ CHECKSUM, -(MAGIC + FLAGS)

# PAGE TABLE ENTRY

.equ PTE_PRESENT,   1<<0
.equ PTE_RW,        1<<1
.equ PTE_US,        1<<2
.equ PTE_G,         1<<8
.equ PTE_ATTR,      (PTE_PRESENT|PTE_RW)

.equ PAGE_SIZE,     4096
.equ CR0_PG,        1<<31
.equ CR4_PAE,       1<<5
.equ LONGMODE,      1<<29

.code32

# MULTIBOOT v1 header for GRUB
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

# XXX: do we need different for kernel?
boot_pml4t: .space 4096,0

boot_pdpt: .space 4096,0
boot_pdt: .space 4096,0
boot_pt: .space 8192,0

boot_kernel_pdpt: .space 4096,0
boot_kernel_pdt: .space 4096,0
boot_kernel_pt: .space 8192,0

# GDTs

gdt64:
.quad 0 # null
gdt64_code:
.long 0xFFFF
.byte 0
.byte ((1<<7) | (1<<4) | (1<<3) | (1<<1))
.byte ((1<<7) | (1<<5) | 0xF)
.byte 0
gdt64_data:
.long 0xFFFF
.byte 0
.byte ((1<<7) | (1<<4) | (1<<1))
.byte ((1<<7) | (1<<6) | 0xF)
.byte 0
gdt64_tss:
.quad 0x00CF890000000068

gdt64ptr:
.word 32
.quad gdt64

.set STACKSIZE, 256
.comm stack, STACKSIZE, 32
.set EFLAGS_ID, 1 << 21

kernel_entrypoint:
    movl $(stack + STACKSIZE-4), %esp
    movl 16(%ebx), %eax
    movl %eax, cmdline

    # check if longmode is available

    call check_cpuid
    call check_longmode

    # enable longmoe

    call set_vmspace
    pushl cmdline
    call enable_longmode

    # NOTREACHABLE

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
    # ERROR
    jmp .

# void check_longmode(void)
check_longmode:
	pushfl
	pushl %eax
	# check if extension is available
	movl $0x80000000, %eax
	cpuid
	cmpl $0x80000001, %eax
	jb no_longmode
	# check if long mode is available
	movl $0x80000001, %eax
	cpuid
	test $LONGMODE, %edx
	jz no_longmode
	popl %eax
	popfl
	ret

    # ERROR
no_longmode:
	jmp .

    # XXX: add information that something went wrong

# void set_vmspace(void)

set_vmspace:

    # CODE ASSUMES THAT OBJECTS ARE ALIGNED TO 4096

    # IDENTITY MAPPING

    # link pml4t[0] with pdpt
    movl $boot_pdpt, %ebx
    orl $PTE_ATTR, %ebx
    movl %ebx, (boot_pml4t)

    # link pdpt[0] with pdt
    movl $boot_pdt, %ebx
    orl $PTE_ATTR, %ebx
    movl %ebx, (boot_pdpt)

    # link pdt[0] with pt[0]
    # link pdt[1] with pt[1]
    movl $boot_pt, %ebx
    orl $PTE_ATTR, %ebx
    movl %ebx, (boot_pdt)
    addl $0x1000, %ebx
    movl $(1), %eax
    movl %ebx, boot_pdt(,%eax,8) 

    # map first 4 MB
    movl $0, %esi
    movl $boot_pt, %edi
    call fill_ptable

    ## KERNEL

    movl $boot_kernel_pdpt, %ebx
    orl $PTE_ATTR, %ebx
    movl $(511), %eax
    movl %ebx, boot_pml4t(,%eax,8)

    movl $boot_kernel_pdt, %ebx
    orl $PTE_ATTR, %ebx
    movl $(510), %eax
    movl %ebx, boot_kernel_pdpt(,%eax,8)

    movl $boot_kernel_pt, %ebx
    orl $PTE_ATTR, %ebx
    movl $0, %eax
    movl %ebx, boot_kernel_pdt(,%eax,8)
    addl $0x1000, %ebx
    movl $1, %eax
    movl %ebx, boot_kernel_pdt(,%eax,8)

    movl $KERNEL_BOOTSTRAP, %esi
    movl $boot_kernel_pt, %edi
    call fill_ptable

    # let cpu know where are mapping tables
    movl $boot_pml4t, %eax
    movl %eax, %cr3
    
    # enable PAE
    movl %cr4, %eax
    orl $CR4_PAE, %eax
    movl %eax, %cr4

    ret

tick:
    movl $0xb8000, %edx
    incw (%edx)
    ret

.global tick

# void fill_ptable(ptable, paddr)
# maps 4MB in two consequent page tables
fill_ptable:
    movl $1024, %ecx
    orl $PTE_ATTR, %esi
.2:
    movl %esi, (%edi)
    addl $PAGE_SIZE, %esi
    addl $8, %edi
    loop .2
    ret

enable_longmode:
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    movl %cr0, %eax
    orl $CR0_PG, %eax
    movl %eax, %cr0
    lgdt (gdt64ptr)
    # set data segs?
    jmp $0x8,$trampoline

trampoline:
.code64
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movabsq $kernel_startup, %rax
    call *%rax
    jmp .

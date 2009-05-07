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

.equ text_selector, 0x08
.equ data_selector, 0x10

.equ utext_selector, 0x18
.equ udata_selector, 0x20
.equ SEL_DPL3, 0x3

.global 
.global cpu_user_mode
.global cpu_gdt_load
.global cpu_ldt_load
.global cpu_idt_load
.global cpu_tr_load
.global cpu_tss_save
.global cpu_jmp_sel
.global thread_context_store
.global thread_context_load
.global cpu_get_cr0
.global cpu_get_cr2
.global cpu_get_cr3
.global cpu_get_cr4
.global cpu_set_cr0
.global cpu_set_cr2
.global cpu_set_cr3
.global cpu_set_cr4


.macro offset32 name, num
.equ \name, \num*4
.endm


offset32    CTX_ESP,    0
offset32    CTX_EBP,    1
offset32    CTX_EFLAGS, 2
offset32    CTX_CR3,    3

cpu_user_mode:
    movl %esp, %eax
    pushl $udata_selector|SEL_DPL3
    pushl %eax
    pushf
    pushl $utext_selector|SEL_DPL3
    pushl $1f
    iret
1:
    mov $udata_selector|SEL_DPL3, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %ss
    mov %ax, %gs
    ret

thread_context_load:
    enter $0, $0
    movl 8(%ebp), %edi
    movl CTX_CR3(%edi), %eax
    movl %eax, %cr3
    movl CTX_ESP(%edi), %esp
    movl CTX_EBP(%edi), %ebp
    pushl CTX_EFLAGS(%edi)
    popfl
    movl $0x0, %eax
    leave
    ret


thread_context_store:
    enter $0, $0
    movl 8(%ebp), %edi
    movl %ebp, CTX_EBP(%edi)
    movl %esp, CTX_ESP(%edi)
    movl %cr3, %eax
    movl %eax, CTX_CR3(%edi)
    pushfl
    popl CTX_EFLAGS(%edi)
    movl $0x1, %eax
    leave
    ret

cpu_gdt_load:
    enter $0,$0
    movl 8(%ebp), %eax
    lgdt (%eax)
    movl $data_selector, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs
    movl %eax, %ss
    ljmp $text_selector, $gdt_load.1
    movl %eax, %cr0
    or $0x1, %eax
    movl %cr0, %eax
gdt_load.1:
    leave
    ret

cpu_ldt_load:
    movl 4(%esp), %eax
    lldt (%eax)
    ret

cpu_idt_load:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

cpu_tr_load:
    ltr 4(%esp)
    ret

cpu_get_cr0:
    movl %cr0, %eax
    ret

cpu_set_cr0:
    movl 4(%esp), %eax
    movl %eax, %cr0
    ret

cpu_get_cr2:
    movl %cr2, %eax
    ret

cpu_set_cr2:
    movl 4(%esp), %eax
    movl %eax, %cr2
    ret

cpu_get_cr3:
    movl %cr3, %eax
    ret

cpu_set_cr3:
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret

cpu_get_cr4:
    movl %cr4, %eax
    ret

cpu_set_cr4:
    movl 4(%esp), %eax
    movl %eax, %cr4
    ret


cpu_jmp_sel:
    movl 4(%esp), %eax
    pushl $0x0
    pushl %eax
    ljmp *-4(%esp)
    addl $8, %esp
    ret

.global far_copy_in
# funkcja kopiujaca pomiedzy wybranym selektorem
# segmentu a obecnie uzywanym selektorem danych
# sluzy do kopiowania np danych z segmentu uzytkownika
# do segmentu jadra
far_copy_in:
    enter $0, $0
    pushl %es   
    pushl %ds
    pushl %edi
    pushl %esi
    movw %ds, %ax
    movw %ax, %es
    movl 8(%ebp), %edi  # dst
    movw 12(%ebp), %ds  # src sel
    movl 16(%ebp), %esi # src
    movl 20(%ebp), %ecx # len
    rep movsb       # skopiuje ECX bajtow z DS:ESI do ES:EDI
    popl %esi
    popl %edi
    popl %ds
    popl %es
    leave
    ret


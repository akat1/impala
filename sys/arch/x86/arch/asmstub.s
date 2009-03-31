# 
#  ImpalaOS
#   http://trzask.codepainters.com/impala/trac/
# 
#  $Id$
#  

.set text_selector, 0x08
.set data_selector, 0x10


.global gdt_load
.global ldt_load
.global idt_load
.global tr_load
.global tss_save
.global jmp_sel
.global thread_context_store
.global thread_context_load

.macro offset32 name, num
.equ \name, \num*4
.endm


offset32    CTX_ESP,    0
offset32    CTX_EBP,    1
offset32    CTX_EFLAGS, 2
offset32    CTX_CR3,    3

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

gdt_load:
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

ldt_load:
	movl 4(%esp), %eax
	lldt (%eax)
	ret

idt_load:
    movl 4(%esp), %eax
	lidt (%eax)
	ret

tr_load:
    ltr 4(%esp)
    ret

jmp_sel:
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
    rep movsb           # skopiuje ECX bajtow z DS:ESI do ES:EDI
    popl %esi
    popl %edi
    popl %ds
    popl %es
    leave
    ret


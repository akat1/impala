#
# $Id$

.global _setjmp
.global _longjmp


.macro offset32 name, num
.equ \name, \num*4
.endm

offset32    JMP_EAX,    0
offset32    JMP_EBX,    1
offset32    JMP_ECX,    2
offset32    JMP_EDX,    3
offset32    JMP_ESI,    4
offset32    JMP_EDI,    5
offset32    JMP_ESP,    6
offset32    JMP_EBP,    7
offset32    JMP_EIP,    8


_setjmp:
    movl 4(%esp), %eax
    movl %ebx, JMP_EBX(%eax)
    movl %ecx, JMP_ECX(%eax)
    movl %edx, JMP_EDX(%eax)
    movl %esi, JMP_ESI(%eax)
    movl %edi, JMP_EDI(%eax)
    movl %ebp, JMP_EBP(%eax)
    movl %esp, JMP_ESP(%eax)
    movl (%esp), %ebx
    movl %ebx, JMP_EIP(%eax)
    movl $0, %eax
    ret

_longjmp:
    movl 8(%esp), %eax
    movl 4(%esp), %ebx
    movl JMP_EDX(%ebx), %edx
    movl JMP_ESI(%ebx), %esi
    movl JMP_EDI(%ebx), %edi
    movl JMP_EBP(%ebx), %ebp
    movl JMP_ESP(%ebx), %esp
    movl JMP_EIP(%ebx), %ecx
    movl %ecx, (%esp)
    movl JMP_ECX(%ebx), %ecx
    movl JMP_EBX(%ebx), %ebx
    ret


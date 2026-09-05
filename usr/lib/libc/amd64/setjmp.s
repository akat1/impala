.global _setjmp
.global _longjmp
.global __setjmp
.global __longjmp
.global setjmp
.global longjmp

.macro offset64 name, num
.equ \name, \num*8
.endm

offset64    JMP_RBX,    0
offset64    JMP_RBP,    1
offset64    JMP_R12,    2
offset64    JMP_R13,    3
offset64    JMP_R14,    4
offset64    JMP_R15,    5
offset64    JMP_RSP,    6
offset64    JMP_RIP,    7

__setjmp:
_setjmp:
setjmp:
    movq %rbx, JMP_RBX(%rdi)
    movq %rbp, JMP_RBP(%rdi)
    movq %r12, JMP_R12(%rdi)
    movq %r13, JMP_R13(%rdi)
    movq %r14, JMP_R14(%rdi)
    movq %r15, JMP_R15(%rdi)
    movq %rsp, JMP_RSP(%rdi)
    movq (%rsp), %rax
    movq %rax, JMP_RIP(%rdi)
    xorq %rax, %rax
    ret

__longjmp:
_longjmp:
longjmp:
    movq %rsi, %rax
    movq JMP_RBX(%rdi), %rbx
    movq JMP_RBP(%rdi), %rbp
    movq JMP_R12(%rdi), %r12
    movq JMP_R13(%rdi), %r13
    movq JMP_R14(%rdi), %r14
    movq JMP_R15(%rdi), %r15
    movq JMP_RSP(%rdi), %rsp
    movq JMP_RIP(%rdi), %rdx
    movq %rdx, (%rsp)
    testq %rax, %rax
    jnz 1f
    movq $1, %rax
1:
    ret

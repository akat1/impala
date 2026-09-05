# Impala Operating System

.code64

.equ text_selector, 0x08
.equ data_selector, 0x10

.equ KSTACKSIZE, 0x16000
.comm kstack, KSTACKSIZE, 32

.global cpu_user_mode
.global cpu_gdt_load
.global cpu_ldt_load
.global cpu_idt_load
.global cpu_tr_load
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
.global cpu_get_eflags
.global kernel_startup
.global kstack
.global cpu_resume

.macro offset64 name, num
.equ \name, \num*8
.endm

offset64    CTX_RAX,    0
offset64    CTX_RBX,    1
offset64    CTX_RCX,    2
offset64    CTX_RDX,    3
offset64    CTX_RSI,    4
offset64    CTX_RDI,    5
offset64    CTX_R8,     6
offset64    CTX_R9,     7
offset64    CTX_R10,    8
offset64    CTX_R11,    9
offset64    CTX_R12,    10
offset64    CTX_R13,    11
offset64    CTX_R14,    12
offset64    CTX_R15,    13
offset64    CTX_RSP,    14
offset64    CTX_RBP,    15
offset64    CTX_RFLAGS, 16
offset64    CTX_CR3,    17
offset64    CTX_RIP,    18

kernel_startup:
    movq 8(%rsp), %rdi
    movq $(kstack + KSTACKSIZE-16), %rsp
    call init_x86
    call kmain
    jmp .

cpu_user_mode:
    ret

# void cpu_resume(interrupt_frame *f);
cpu_resume:
    movq %rdi, %rsp
    popq %rax
    movw %ax, %gs
    popq %rax
    movw %ax, %fs
    popq %rax
    movw %ax, %es
    popq %rax
    movw %ax, %ds
    addq $8, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
    addq $16, %rsp
    sti
    iretq

thread_context_store:
    movq %rbx, CTX_RBX(%rdi)
    movq %rcx, CTX_RCX(%rdi)
    movq %rdx, CTX_RDX(%rdi)
    movq %rsi, CTX_RSI(%rdi)
    movq %r8, CTX_R8(%rdi)
    movq %r9, CTX_R9(%rdi)
    movq %r10, CTX_R10(%rdi)
    movq %r11, CTX_R11(%rdi)
    movq %r12, CTX_R12(%rdi)
    movq %r13, CTX_R13(%rdi)
    movq %r14, CTX_R14(%rdi)
    movq %r15, CTX_R15(%rdi)
    movq %rbp, CTX_RBP(%rdi)
    movq %rsp, CTX_RSP(%rdi)
    movq (%rsp), %rax
    movq %rax, CTX_RIP(%rdi)
    movq %cr3, %rax
    movq %rax, CTX_CR3(%rdi)
    pushfq
    popq CTX_RFLAGS(%rdi)
    movq $1, %rax
    ret

thread_context_load:
    movq %rdi, %rbx
    movq CTX_RDX(%rbx), %rdx
    movq CTX_RSI(%rbx), %rsi
    movq CTX_R8(%rbx), %r8
    movq CTX_R9(%rbx), %r9
    movq CTX_R10(%rbx), %r10
    movq CTX_R11(%rbx), %r11
    movq CTX_R12(%rbx), %r12
    movq CTX_R13(%rbx), %r13
    movq CTX_R14(%rbx), %r14
    movq CTX_R15(%rbx), %r15
    movq CTX_RBP(%rbx), %rbp
    movq CTX_RSP(%rbx), %rsp
    movq CTX_RIP(%rbx), %rcx
    movq %rcx, (%rsp)
    movq CTX_RCX(%rbx), %rcx
    pushq CTX_RFLAGS(%rbx)
    popfq
    movq CTX_CR3(%rbx), %rax
    movq %rax, %cr3
    movq CTX_RBX(%rbx), %rbx
    xorq %rax, %rax
    ret

cpu_gdt_load:
    lgdt (%rdi)
    movq $data_selector, %rax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ret

cpu_ldt_load:
    lldt (%rdi)
    ret

cpu_idt_load:
    lidt (%rdi)
    ret

cpu_tr_load:
    ltr %di
    ret

cpu_get_cr0:
    movq %cr0, %rax
    ret

cpu_set_cr0:
    movq %rdi, %cr0
    ret

cpu_get_cr2:
    movq %cr2, %rax
    ret

cpu_set_cr2:
    movq %rdi, %cr2
    ret

cpu_get_cr3:
    movq %cr3, %rax
    ret

cpu_set_cr3:
    movq %rdi, %cr3
    ret

cpu_get_cr4:
    movq %cr4, %rax
    ret

cpu_set_cr4:
    movq %rdi, %cr4
    ret

cpu_get_eflags:
    pushfq
    popq %rax
    ret

cpu_jmp_sel:
    movzwq %di, %rax
    pushq %rax
    leaq 1f(%rip), %rax
    pushq %rax
    lretq
1:
    ret

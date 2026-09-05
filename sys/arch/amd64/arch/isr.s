# Impala Operating System

.text
.code64

.global _intrpt_syscall
.global _unhnd_intrpt

.set data_selector, 0x10

.macro PUSH_REGS
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq $0
.endm

.macro POP_REGS
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
.endm

.macro PUSH_SEGS
    xorq %rax, %rax
    movw %ds, %ax
    pushq %rax
    xorq %rax, %rax
    movw %es, %ax
    pushq %rax
    xorq %rax, %rax
    movw %fs, %ax
    pushq %rax
    xorq %rax, %rax
    movw %gs, %ax
    pushq %rax
.endm

.macro POP_SEGS
    popq %rax
    movw %ax, %gs
    popq %rax
    movw %ax, %fs
    popq %rax
    movw %ax, %es
    popq %rax
    movw %ax, %ds
.endm

.macro SET_KERNEL_REGS
    movq $data_selector, %rax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
.endm

.macro TRAP_ERR num, name, handler
\name:
    cli
    pushq $\num
    PUSH_REGS
    PUSH_SEGS
    SET_KERNEL_REGS
    movq %rsp, %rdi
    call \handler
    POP_SEGS
    POP_REGS
    addq $16, %rsp
    sti
    iretq
.endm

.macro TRAP_NOERR num, name, handler
\name:
    cli
    pushq $0
    pushq $\num
    PUSH_REGS
    PUSH_SEGS
    SET_KERNEL_REGS
    movq %rsp, %rdi
    call \handler
    POP_SEGS
    POP_REGS
    addq $16, %rsp
    sti
    iretq
.endm

.macro ISR num, name, handler
\name:
    cli
    pushq $0
    pushq $\num
    PUSH_REGS
    PUSH_SEGS
    SET_KERNEL_REGS
    movq %rsp, %rdi
    call \handler
    POP_SEGS
    POP_REGS
    addq $16, %rsp
    sti
    iretq
.endm

TRAP_NOERR 0, _unhnd_intrpt, TRAP_unhandled

TRAP_NOERR 0, _trap0, TRAP_unhandled
TRAP_NOERR 1, _trap1, TRAP_unhandled
TRAP_NOERR 2, _trap2, TRAP_unhandled
TRAP_NOERR 3, _trap3, TRAP_unhandled
TRAP_NOERR 4, _trap4, TRAP_unhandled
TRAP_NOERR 5, _trap5, TRAP_unhandled
TRAP_NOERR 6, _trap6, TRAP_unhandled
TRAP_NOERR 7, _trap7, TRAP_unhandled
TRAP_ERR 8, _trap8, TRAP_unhandled
TRAP_NOERR 9, _trap9, TRAP_unhandled
TRAP_ERR 10, _trap10, TRAP_unhandled
TRAP_ERR 11, _trap11, TRAP_unhandled
TRAP_ERR 12, _trap12, TRAP_unhandled
TRAP_ERR 13, _trap13, TRAP_gfault
TRAP_ERR 14, _trap14, TRAP_pfault
TRAP_NOERR 15, _trap15, TRAP_unhandled
TRAP_NOERR 16, _trap16, TRAP_unhandled

TRAP_NOERR 80, _intrpt_syscall, ISR_syscall

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

irq_table:
    .quad _isr0
    .quad _isr1
    .quad _isr2
    .quad _isr3
    .quad _isr4
    .quad _isr5
    .quad _isr6
    .quad _isr7
    .quad _isr8
    .quad _isr9
    .quad _isr10
    .quad _isr11
    .quad _isr12
    .quad _isr13
    .quad _isr14
    .quad _isr15
    .quad _isr16
    .quad _isr17
    .quad _isr18
    .quad _isr19
    .quad _isr20
    .quad _isr21
    .quad _isr22
    .quad _isr23
    .quad 0

trap_table:
    .quad _trap0
    .quad _trap1
    .quad _trap2
    .quad _trap3
    .quad _trap4
    .quad _trap5
    .quad _trap6
    .quad _trap7
    .quad _trap8
    .quad _trap9
    .quad _trap10
    .quad _trap11
    .quad _trap12
    .quad _trap13
    .quad _trap14
    .quad _trap15
    .quad _trap16
    .quad 0

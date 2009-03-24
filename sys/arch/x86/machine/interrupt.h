#ifndef __MACHINE_INTERRUPT_H
#define __MACHINE_INTERRUPT_H


enum {
    INTERRUPT_VECTOR = 0x20,
    MAX_IRQ = 23,
    INTRPT_SCHED = 0x70,
    INTRPT_SYSCALL = 0x80
};

enum {
    IRQ0 = 0,
    IRQ1,
    IRQ2,
    IRQ3,
    IRQ4,
    IRQ5,
    IRQ6,
    IRQ7,
    IRQ8,
    IRQ9,
    IRQ10,
    IRQ11,
    IRQ12,
    IRQ13
    // ...
};


typedef bool irq_handler_f(void);

typedef struct interrupt_frame interrupt_frame;
struct interrupt_frame {
    // pusha
    uint32_t f_edi;
    uint32_t f_esi;
    uint32_t f_ebp;
    uint32_t f_isp;
    uint32_t f_ebx;
    uint32_t f_edx;
    uint32_t f_ecx;
    uint32_t f_eax;

    // recznie
    uint32_t f_gs;
    uint32_t f_fs;
    uint32_t f_es;
    uint32_t f_ds;

    uint32_t f_n;
    uint32_t f_errno;
    uint32_t f_eip;
    uint32_t f_cs;

    uint32_t f_eflags;
    uint32_t f_esp;
    uint32_t f_ss;
} __packed;

#ifdef __KERNEL
void irq_install_handler(int irq, irq_handler_f *h);
void irq_free_handler(int irq);
void irq_done(void);


/// Wy³±cza obs³ugê przerwañ
static inline void irq_disable(void)
{
    __asm__ ("cli");
}

/// W³±cza obs³ugê przerwañ przez procesor
static inline void irq_enable(void)
{
    __asm__ ("sti");
}

#endif


#endif

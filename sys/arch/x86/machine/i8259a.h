/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_i8259A_H
#define __MACHINE_i8259A_H

enum {
    PIC_IRQ0 = 0x01,
    PIC_IRQ1 = 0x02,
    PIC_IRQ2 = 0x04,
    PIC_IRQ3 = 0x08,
    PIC_IRQ4 = 0x10,
    PIC_IRQ5 = 0x20,
    PIC_IRQ6 = 0x40,
    PIC_IRQ7 = 0x80
};

enum {
    PIC_IRQ8 = 0x01,
    PIC_IRQ9 = 0x02,
    PIC_IRQ10 = 0x04,
    PIC_IRQ11 = 0x08,
    PIC_IRQ12 = 0x10,
    PIC_IRQ13 = 0x20,
    PIC_IRQ14 = 0x40,
    PIC_IRQ15 = 0x80
};

#ifdef __KERNEL

void i8259a_init(void);
void i8259a_send_eoi(void);
void i8259a_reset_mask(void);
void i8259a_irq_enable(int n);
void i8259a_irq_disable(int n);

#endif

#endif

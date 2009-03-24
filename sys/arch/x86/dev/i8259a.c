#include <sys/types.h>
#include <machine/i8259a.h>
#include <machine/io.h>
#include <machine/interrupt.h>

enum {
    PIC_M = 0x20,
    PIC_S = 0xA0
};

enum {
    ICW1_ICW4       = 0x01,
    ICW1_CASCADE    = 0x02,
    ICW1_INTERVAL8  = 0x00,
    ICW1_INTERVAL4  = 0x04,
    ICW1_TRIG_EDGE  = 0x00,
    ICW1_TRIG_LEVEL = 0x08,
    ICW1_RESET      = 0x10
};


enum {
    ICW4_8086          = 0x01,
    ICW4_EOI_AUTO      = 0x02,
    ICW4_EOI_NORMAL    = 0x00,
    ICW4_BUF_NONE      = 0x00,
    ICW4_BUF_MASTER    = 0x00,
    ICW4_BUF_SLAVE     = 0x00,
    ICW4_NESTED        = 0x80
};

enum {
    OCW2_EOI_NORMAL = 0x20
};

static uchar pic1_mask;
static uchar pic2_mask;


void
i8259a_init()
{
    // ICW1
    io_out8(PIC_M, ICW1_RESET | ICW1_ICW4);
    // ICW2
    io_out8(PIC_M+1, INTERRUPT_VECTOR);   
    // ICW3
    io_out8(PIC_M+1, 0x04);
    // ICW4
    io_out8(PIC_M+1, ICW4_8086);

    // ICW1
    io_out8(PIC_S, ICW1_RESET | ICW1_ICW4);
    // ICW2
    io_out8(PIC_S+1, INTERRUPT_VECTOR+0x8);   
    // ICW3
    io_out8(PIC_S+1, 0x02);
    // ICW4
    io_out8(PIC_S+1, ICW4_8086);

    pic1_mask = 0x0;
    pic2_mask = 0x0;
    i8259a_reset_mask();
}


void
i8259a_reset_mask()
{
    io_out8(PIC_M+1, ~pic1_mask);
    io_out8(PIC_S+1, ~pic2_mask);
}

void
i8259a_irq_enable(int n)
{
    if (n < 0x8)
        pic1_mask |= 1 << n;
    else
        pic2_mask |= 1 << (n-8);
    i8259a_reset_mask();
}

void
i8259a_irq_disable(int n)
{
    if (n < 0x8)
        pic1_mask &= ~(1 << n);
    else
        pic2_mask &= ~(1 << (n-8));
    i8259a_reset_mask();
}

void
i8259a_send_eoi()
{
    io_out8(PIC_M, OCW2_EOI_NORMAL);
    io_out8(PIC_S, OCW2_EOI_NORMAL);
}


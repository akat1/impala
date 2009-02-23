#include <sys/types.h>
#include <machine/descriptor.h>
#include <machine/tss.h>
#include <machine/i8259a.h>
#include <machine/i8254.h>
#include <machine/interrupt.h>
#include <machine/cpu.h>
#include <sys/libkutil.h>
#include <sys/thread.h>

void init_x86(void);
void _unhnd_intrpt(void);
void _intrpt_syscall(void);

extern uintptr_t trap_table[];
extern uintptr_t irq_table[];

static void setgdt(int, uintptr_t, uint, uint, uint);
static void setidt(int i, uint selector, uintptr_t offset, uint access);
static descriptor p_gdt[SEL_MAX];
static descriptor p_idt[0x100];
static task_state_segment p_tss0;
static descriptor_register p_gdtr;
static descriptor_register p_idtr;


void
init_x86()
{
    enum {
        code = GATE_PRESENT | GATE_TYPE_RX,
        ucode = code | GATE_DPL3,
        data = GATE_PRESENT | GATE_TYPE_RW,
        udata = data | GATE_DPL3,
        attr = GATE_PAGEGRAN | GATE_OP32,
        tss0 = GATE_PRESENT | GATE_TYPE_TASK,
        intrpt_attr = GATE_PRESENT | GATE_TYPE_INTRPT,
        trap_attr = GATE_PRESENT | GATE_TYPE_TRAP
    };
    int i;

    // Ustawienie GDT
    mem_zero(&p_gdt, sizeof(p_gdt));
    mem_zero(&p_tss0, sizeof(p_tss0));
    setgdt(SEL_CODE, 0x0, 0xfffff, code, attr);
    setgdt(SEL_DATA, 0x0, 0xfffff, data, attr);
    setgdt(SEL_UCODE, 0x0, 0xffff, ucode, attr);
    setgdt(SEL_UDATA, 0x0, 0xffff, udata, attr);
    setgdt(SEL_TSS0, (uintptr_t)&p_tss0, sizeof(p_tss0), tss0, 0);

    mem_zero(&p_gdtr, sizeof(p_gdtr));
    p_gdtr.base = &p_gdt;
    p_gdtr.limit = sizeof(p_gdt) -1;
    gdt_load(&p_gdtr);

    tr_load(SEL_MK(SEL_TSS0, SEL_DPL0));

    // Ustawienie IDT
    for (i = 0; i < 0x100; i++) {
        setidt(i, SEL_MK(SEL_CODE, SEL_DPL0), (uintptr_t)&_unhnd_intrpt, intrpt_attr);
    }

    for (i = 0; i < 0x20; i++) {
        setidt(i, SEL_MK(SEL_CODE, SEL_DPL0), trap_table[i], trap_attr);
    }

    for (i = 0; i <= 23; i++) {
        setidt(i+0x20, SEL_MK(SEL_CODE, SEL_DPL0), irq_table[i], intrpt_attr);
    }

    setidt(INTRPT_SYSCALL, SEL_MK(SEL_CODE, SEL_DPL3), (uintptr_t)_intrpt_syscall, intrpt_attr);

    mem_zero(&p_idtr, sizeof(p_idtr));
    p_idtr.base = &p_idt;
    p_idtr.limit = sizeof(p_idt)-1;
    idt_load(&p_idtr);
    i8259a_init();
    i8254_init();
    __asm__("sti");

}

void
setidt(int i, uint selector, uintptr_t offset,
    uint access)
{
    gate_descr *entry = &p_idt[i].gdescr;
    entry->offset_low = (uint)offset & 0xffff;
    entry->offset_high = ((uint)offset >> 16);
    entry->selector = selector;
    entry->notused = 0;
    entry->access = access;
}


void
setgdt(int i, uintptr_t base, uint limit,
    uint acc, uint atrib)
{
    segment_descr *entry = &p_gdt[i].sdescr;
    entry->base_low = base & 0xffff;
    entry->base_mid = (base >> 16) & 0xff;
    entry->base_high = (base >> 24);
    entry->limit_low = limit & 0xffff;
    entry->attr = (limit >> 16) & 0xff;
    entry->attr |= atrib;
    entry->access = acc;
}


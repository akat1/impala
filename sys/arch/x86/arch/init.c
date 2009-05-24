/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/utils.h>
#include <sys/thread.h>
#include <sys/vm.h>
#include <sys/string.h>

#include <machine/descriptor.h>
#include <machine/tss.h>
#include <machine/i8259a.h>
#include <machine/i8254.h>
#include <machine/interrupt.h>
#include <machine/cpu.h>
#include <machine/pckbd.h>
#include <machine/video.h>

void kmain(void);
void init_x86(void);
void _cpuid(int option, struct cpuid_result *r);
void _unhnd_intrpt(void);
void _intrpt_syscall(void);
void _cpu_info(void);

extern uintptr_t trap_table[];
extern uintptr_t irq_table[];
struct cpu_info cpu_i;

static void setgdt(int, uintptr_t, uint, uint, uint);
static void setidt(int i, uint selector, uintptr_t offset, uint access);
static descriptor_t p_gdt[SEL_MAX];
static descriptor_t p_idt[0x100];
static task_state_segment p_tss0;
static descriptor_register_t p_gdtr;
static descriptor_register_t p_idtr;

void
_cpuid(int option, struct cpuid_result *r) {

    __asm__ volatile (
        "cpuid;"
        :"=a"(r->r_eax), "=b"(r->r_ebx), "=c"(r->r_ecx), "=d"(r->r_edx)
        :"a"(option));

    return;
}

void
_cpu_info(void)
{
    struct cpuid_result cpuid_r;

    mem_zero(&cpu_i, sizeof(struct cpu_info));

    _cpuid(CPUID_BASIC ,&cpuid_r);

    ((unsigned int *)&(cpu_i.vendor_string))[0] = cpuid_r.r_ebx;
    ((unsigned int *)&(cpu_i.vendor_string))[1] = cpuid_r.r_edx;
    ((unsigned int *)&(cpu_i.vendor_string))[2] = cpuid_r.r_ecx;

    _cpuid(CPUID_FEATURE ,&cpuid_r);

    cpu_i.version_information = cpuid_r.r_eax;
    cpu_i.feature_ecx = cpuid_r.r_ecx;
    cpu_i.feature_edx = cpuid_r.r_edx;
}

/**
 * Inicjalizuje obs³ugê architektury x86.
 *
 * Procedura jest wywo³ywana przez procedurê wej¶ciow±
 * j±dra. Jej zadanie to zainicjalizowanie ¶rodowiska
 * do pracy w systemie operacyjnym.
 */

void setesp0(void *a);

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
    setgdt(SEL_UCODE, 0x0, 0xfffff, ucode, attr);
    setgdt(SEL_UDATA, 0x0, 0xfffff, udata, attr);
    setgdt(SEL_TSS0, (uintptr_t)&p_tss0, sizeof(p_tss0), tss0, 0);
    p_tss0.tss_io = 0;
    p_tss0.tss_ss0 = 0x10;
//    p_tss0.tss_cs=0x8;
//    p_tss0.tss_ds=p_tss0.tss_es=p_tss0.tss_fs=p_tss0.tss_gs=0x10;

    mem_zero(&p_gdtr, sizeof(p_gdtr));
    p_gdtr.base = &p_gdt;
    p_gdtr.limit = sizeof(p_gdt) -1;
    cpu_gdt_load(&p_gdtr);

    cpu_tr_load(SEL_MK(SEL_TSS0, SEL_DPL3));
    extern void megaloop(void);

    // Ustawienie IDT
    for (i = 0; i < 0x100; i++) {
        setidt(i, SEL_MK(SEL_CODE, SEL_DPL0), (uintptr_t)&megaloop,//s_unhnd_intrpt,
            intrpt_attr);
    }

    for (i = 0; i < 0x20; i++) {
        setidt(i, SEL_MK(SEL_CODE, SEL_DPL0), (uintptr_t)trap_table[i], trap_attr);
    }

    for (i = 0; i <= 23; i++) {
        setidt(i+0x20, SEL_MK(SEL_CODE, SEL_DPL0), irq_table[i], intrpt_attr);
    }

    setidt(INTRPT_SYSCALL, SEL_MK(SEL_CODE, SEL_DPL0),
        (uintptr_t)_intrpt_syscall, intrpt_attr | GATE_DPL3);

    mem_zero(&p_idtr, sizeof(p_idtr));
    p_idtr.base = &p_idt;
    p_idtr.limit = sizeof(p_idt)-1;
    cpu_idt_load(&p_idtr);
    i8259a_init();
    i8254_init();
    pckbd_init();
    vm_low_init();
    video_init();
    irq_enable();
    _cpu_info();
}

void
setesp0(void *a)
{
     p_tss0.tss_esp0 = (uint32_t)a;
}


/// Ustawia wpis w tablicy deskryptorów IDT.
void
setidt(int i, uint selector, uintptr_t offset,
    uint access)
{
//     offset = offset - 0xc0000000 + 0x100000;
    gate_descr_t *entry = &p_idt[i].gdescr;
    entry->offset_low = (uint)offset & 0xffff;
    entry->offset_high = ((uint)offset >> 16);
    entry->selector = selector;
    entry->notused = 0;
    entry->access = access;
}

/// Ustawia wpis w tablicy deskryptorów GDT.
void
setgdt(int i, uintptr_t base, uint limit,
    uint acc, uint atrib)
{
//     base = base - 0xc0000000 + 0x100000;
    segment_descr_t *entry = &p_gdt[i].sdescr;
    entry->base_low = base & 0xffff;
    entry->base_mid = (base >> 16) & 0xff;
    entry->base_high = (base >> 24);
    entry->limit_low = limit & 0xffff;
    entry->attr = (limit >> 16) & 0xff;
    entry->attr |= atrib;
    entry->access = acc;
}


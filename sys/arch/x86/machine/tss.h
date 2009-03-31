/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_TSS_H
#define __MACHINE_TSS_H

typedef struct task_state_segment task_state_segment;
struct task_state_segment {
    uint32_t tss_backlink;
    uint32_t tss_esp0;
    uint32_t tss_ss0;
    uint32_t tss_esp1;
    uint32_t tss_ss1;
    uint32_t tss_esp2;
    uint32_t tss_ss2;
    uint32_t tss_cr3;
    uint32_t tss_eip;
    uint32_t tss_eflags;
    uint32_t tss_eax;
    uint32_t tss_ecx;
    uint32_t tss_edx;
    uint32_t tss_ebx;
    uint32_t tss_esp;
    uint32_t tss_ebp;
    uint32_t tss_esi;
    uint32_t tss_edi;
    uint32_t tss_es;
    uint32_t tss_cs;
    uint32_t tss_ss;
    uint32_t tss_ds;
    uint32_t tss_fs;
    uint32_t tss_gs;
    uint32_t tss_ldtr;
    uint32_t tss_io;
} __packed;


#endif



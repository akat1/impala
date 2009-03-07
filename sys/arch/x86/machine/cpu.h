/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_CPU_H
#define __MACHINE_CPU_H

enum CPU_EFLAGS {
    EFLAGS_CS   = 1,
    EFLAGS_PF   = 1 << 2,
    EFLAGS_AF   = 1 << 4,
    EFLAGS_ZF   = 1 << 6,
    EFLAGS_SF   = 1 << 7,
    EFLAGS_TF   = 1 << 8,
    EFLAGS_IF   = 1 << 9,
    EFLAGS_DF   = 1 << 10,
    EFLAGS_OF   = 1 << 11,
    EFLAGS_IOPL = (1 << 12) | (1 << 13),
    EFLAGS_NT   = 1 << 14,
    EFLAGS_RF   = 1 << 16,
    EFLAGS_VM   = 1 << 17,
    EFLAGS_AC   = 1 << 18,
    EFLAGS_VIF  = 1 << 19,
    EFLAGS_VIP  = 1 << 20,
    EFLAGS_ID   = 1 << 21
};

enum CPU_EFLAGS_IOPL {
    EFLAGS_DPL0 = 0,
    EFLAGS_DPL1 = 1 << 12,
    EFLAGS_DPL2 = 2 << 12,
    EFLAGS_DPL3 = 3 << 12
};
#define EFLAGS_BITS 0x2


// rejestry kontrolne
enum CPU_CR0 {
    CR0_PE = 1 << 0,
    CR0_MP = 1 << 1,
    CR0_EM = 1 << 2,
    CR0_TS = 1 << 3,
    CR0_ET = 1 << 4,
    CR0_NE = 1 << 5,
    CR0_WP = 1 << 16,
    CR0_AM = 1 << 18,
    CR0_NW = 1 << 29,
    CR0_CD = 1 << 30,
    CR0_PG = 1 << 31
};

enum CPU_CR3 {
    CR3_PWT = 1 << 3,
    CR3_PCD = 1 << 4
};

enum CPU_CR4 {
    CR4_VME = 1 << 0,
    CR4_PVI = 1 << 1,
    CR4_TSD = 1 << 2,
    CR4_DE  = 1 << 3,
    CR4_PSE = 1 << 4,
    CR4_PAE = 1 << 5,
    CR4_MCE = 1 << 6,
    CR4_PGE = 1 << 7,
    CR4_PCE = 1 << 8,
    CR4_VMXE= 1 << 13,
    CR4_SMXE= 1 << 14
};


#endif




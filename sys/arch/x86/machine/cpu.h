/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_CPU_H
#define __MACHINE_CPU_H


/// Opis bit�w rejestru EFLAGS
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


/// Opis bit�w rejestru CR0
enum CPU_CR0 {
    /// tryb chroniony (protected mode)
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
    /// stronicowanie (paging)
    CR0_PG = 1 << 31
};

/// Opis bit�w rejestru CR3
enum CPU_CR3 {
    CR3_PWT = 1 << 3,
    CR3_PCD = 1 << 4
};

/// Opis bit�w rejestru CR4
enum CPU_CR4 {
    CR4_VME = 1 << 0,
    CR4_PVI = 1 << 1,
    CR4_TSD = 1 << 2,
    CR4_DE  = 1 << 3,
    /// Page Size Extension
    CR4_PSE = 1 << 4,
    /// Physical Address Extension
    CR4_PAE = 1 << 5,
    CR4_MCE = 1 << 6,
    CR4_PGE = 1 << 7,
    CR4_PCE = 1 << 8,
    CR4_VMXE= 1 << 13,
    CR4_SMXE= 1 << 14
};

/** 
 * Opis mo�liwo�ci procesora
 *
 * Intel IA-32 ADSM - 2A - 3-192
 */

#define KASSERT(x) if(!(x)) \
    panic("Assertion ( %s ) failed in file: %s:%u, in function: %s", #x, __FILE__, __LINE__,  __func__);

enum CPU_EDX_FEATURE {
    EDX_FEATURE_FPU   = 1 << 0,
    EDX_FEATURE_VME   = 1 << 1,
    EDX_FEATURE_DE    = 1 << 2,
    EDX_FEATURE_PSE   = 1 << 3,
    EDX_FEATURE_TSC   = 1 << 4,
    EDX_FEATURE_MSR   = 1 << 5,
    EDX_FEATURE_PAE   = 1 << 6,
    EDX_FEATURE_MCE   = 1 << 7,
    EDX_FEATURE_CX8   = 1 << 8,
    EDX_FEATURE_APIC  = 1 << 9,
    EDX_FEATURE_SEP   = 1 << 11,
    EDX_FEATURE_MTRR  = 1 << 12,
    EDX_FEATURE_PGE   = 1 << 13,
    EDX_FEATURE_MCA   = 1 << 14,
    EDX_FEATURE_CMOV  = 1 << 15,
    EDX_FEATURE_PAT   = 1 << 16,
    EDX_FEATURE_PSE36 = 1 << 17,
    EDX_FEATURE_PSN   = 1 << 18,
    EDX_FEATURE_CLFSH = 1 << 19,
    EDX_FEATURE_DS    = 1 << 21,
    EDX_FEATURE_ACPI  = 1 << 22,
    EDX_FEATURE_MMX   = 1 << 23,
    EDX_FEATURE_FXSR  = 1 << 24,
    EDX_FEATURE_SSE   = 1 << 25,
    EDX_FEATURE_SSE2  = 1 << 26,
    EDX_FEATURE_SS    = 1 << 27,
    EDX_FEATURE_HTT   = 1 << 28,
    EDX_FEATURE_TM    = 1 << 29,
    EDX_FEATURE_PBE   = 1 << 31
};

#ifdef __KERNEL
uint32_t cpu_get_cr0(void);
uint32_t cpu_get_cr2(void);
uint32_t cpu_get_cr3(void);
void cpu_set_cr0(uint32_t r);
void cpu_set_cr2(uint32_t r);
void cpu_set_cr3(uint32_t r);

// Vendor
//static char vendor_string[13];

#endif

#endif




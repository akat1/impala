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

#ifndef __MACHINE_CPU_H
#define __MACHINE_CPU_H


/// Opis bitów rejestru EFLAGS
enum CPU_EFLAGS {
    EFLAGS_CS   = 1 << 1,
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


/// Opis bitów rejestru CR0
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
    CR0_NW = 1 << 29,   ///< Not Write Through
    CR0_CD = 1 << 30,   ///< Cache Disable
    CR0_PG = 1 << 31    ///< Paging
};

/// Opis bitów rejestru CR3
enum CPU_CR3 {
    CR3_PWT = 1 << 3,
    CR3_PCD = 1 << 4
};

/// Opis bitów rejestru CR4
enum CPU_CR4 {
    CR4_VME = 1 << 0,
    CR4_PVI = 1 << 1,
    CR4_TSD = 1 << 2,
    CR4_DE  = 1 << 3,
    CR4_PSE = 1 << 4,
    CR4_PAE = 1 << 5,
    CR4_MCE = 1 << 6,
    CR4_PGE = 1 << 7,   ///< Page Global Enable
    CR4_PCE = 1 << 8,
    CR4_VMXE= 1 << 13,
    CR4_SMXE= 1 << 14
};

/**
 * Opis możliwości procesora
 *
 * Intel IA-32 ADSM - 2A - 3-192 - ECX
 * Intel IA-32 ADSM - 2A - 3-192 - EDX
 */

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

enum CPU_ECX_FEATURE {
    ECX_FEATURE_SSE3        = 1 << 0,
    ECX_FEATURE_MONITOR     = 1 << 3,
    ECX_FEATURE_DSCPL       = 1 << 4,
    ECX_FEATURE_VMX         = 1 << 5,
    ECX_FEATURE_SMX         = 1 << 6,
    ECX_FEATURE_EST         = 1 << 7,
    ECX_FEATURE_TM2         = 1 << 8,
    ECX_FEATURE_SSSE3       = 1 << 9,
    ECX_FEATURE_CNXTID      = 1 << 10,
    ECX_FEATURE_CMPXCHG16B  = 1 << 13,
    ECX_FEATURE_XTPR        = 1 << 14,
    ECX_FEATURE_PDCM        = 1 << 15,
    ECX_FEATURE_DCA         = 1 << 18,
    ECX_FEATURE_SSE41       = 1 << 19,
    ECX_FEATURE_SSE42       = 1 << 20,
    ECX_FEATURE_POPCNT      = 1 << 23
};


enum CPU_CPUID_INDEX {
    CPUID_BASIC         = 0x00,
    CPUID_FEATURE       = 0x01,
    CPUID_CACHE_TLB     = 0x02,
    CPUID_EXT_MAX       = 0x80000000,
    CPUID_EXT_FEATURE   = 0x80000001,
    CPUID_EXT_BRAND1    = 0x80000002,
    CPUID_EXT_BRAND2    = 0x80000003,
    CPUID_EXT_BRAND3    = 0x80000004
};



#ifdef __KERNEL
struct interrupt_frame;

void cpu_user_mode(void);
uint32_t cpu_get_cr0(void);
uint32_t cpu_get_cr2(void);
uint32_t cpu_get_cr3(void);
uint32_t cpu_get_cr4(void);
void cpu_set_cr0(uint32_t r);
void cpu_set_cr2(uint32_t r);
void cpu_set_cr3(uint32_t r);
void cpu_set_cr4(uint32_t r);
uint32_t cpu_get_eflags(void);

void cpu_resume(struct interrupt_frame *);

struct cpuid_result {
    unsigned int r_eax;
    unsigned int r_ebx;
    unsigned int r_ecx;
    unsigned int r_edx;
};

struct cpu_info {
    char vendor_string[13];
    char brand_string[54];
    unsigned int version_information;
    unsigned int feature_ecx;
    unsigned int feature_edx;
};

extern struct cpu_info cpu_i;

#endif

#endif




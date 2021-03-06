/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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

#ifndef __MACHINE_DESCRIPTOR_H
#define __MACHINE_DESCRIPTOR_H


typedef struct segment_descr segment_descr_t;

/// Deskryptor segmentu.
struct segment_descr {
    uint16_t    limit_low;
    uint16_t    base_low;
    uint8_t     base_mid;
    uint8_t     access;
    uint8_t     attr;
    uint8_t     base_high;
} __packed;

typedef struct gate_descr gate_descr_t;
/// Deskryptor bramy
struct gate_descr {
    uint16_t    offset_low;
    uint16_t    selector;
    uint8_t     notused;
    uint8_t     access;
    uint16_t    offset_high;
} __packed;

typedef union descriptor descriptor_t;
/// Deskryptor.
union descriptor {
    segment_descr_t   sdescr;
    gate_descr_t      gdescr;
};

/// Indeksy GDT.
enum {
    SEL_NULL,
    /// kod jądra.
    SEL_CODE,
    /// dane jądra.
    SEL_DATA,
    /// kod użytkownika.
    SEL_UCODE,
    /// dane użytkownika
    SEL_UDATA,
    /// zadanie jądra.
    SEL_TSS0,
    /// zadanie użytkownika.
//    SEL_UTSS0,
    /// bramka do trybu rzeczywistego
//    SEL_VM86,
    SEL_MAX
};

typedef struct descriptor_register descriptor_register_t;
/// Opis rejestrów GDT i LDT.
struct descriptor_register {
  uint16_t  limit;
  addr_t    base;
} __packed;

#define SEL_MK(sel, flags) ((sel * 8) | flags)

enum { 
    SEL_DPL0    = 0x00,
    SEL_DPL1    = 0x01,
    SEL_DPL2    = 0x02,
    SEL_DPL3    = 0x03
};

#define SEL_DPL0 0x0
#define SEL_DPL1 0x1
#define SEL_DPL2 0x2
#define SEL_DPL3 0x3

enum {
    GATE_PRESENT    = 0x80,
    GATE_PAGEGRAN   = 0x80,
    GATE_OP32       = 0x40
};

enum {
    GATE_DPL0   = 0x00,
    GATE_DPL1   = 0x20,
    GATE_DPL2   = 0x40,
    GATE_DPL3   = 0x60
};

enum {
    GATE_TYPE_TASK      = 0x09,
    GATE_TYPE_INTRPT    = 0x0e,
    GATE_TYPE_TRAP      = 0x0f,
    GATE_TYPE_DATA      = 0x10,
    GATE_TYPE_CODE      = 0x18,

    // R - read
    // W - write
    // X - exec
    // C - conform
    // D - expand down
    // order: RWXCD
    GATE_TYPE_X         = GATE_TYPE_CODE,
    GATE_TYPE_RX        = GATE_TYPE_X  | 0x02,
    GATE_TYPE_XC        = GATE_TYPE_X  | 0x04,
    GATE_TYPE_RXC       = GATE_TYPE_RX | 0x04,
    GATE_TYPE_R         = GATE_TYPE_DATA,
    GATE_TYPE_RD        = GATE_TYPE_R  | 0x04,
    GATE_TYPE_RW        = GATE_TYPE_R  | 0x02,
    GATE_TYPE_RWD       = GATE_TYPE_RW | 0x04
};

#ifdef __KERNEL
void cpu_gdt_load(descriptor_register_t *r);
void cpu_ldt_load(descriptor_register_t *r);
void cpu_idt_load(descriptor_register_t *r);
void cpu_tr_load(uint32_t sel);
void cpu_jmp_sel(uint32_t sel);
#endif

#endif

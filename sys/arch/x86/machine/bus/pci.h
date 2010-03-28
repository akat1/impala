/* Impala Operating System
 *
 * Copyright (C) 2010 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
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

#ifndef __MACHINE_BUS_PCI_H
#define __MACHINE_BUS_PCI_H
#ifdef __KERNEL


enum {
    CONFIG_ADDRES   = 0xCF8,
    CONFIG_DATA     = 0xCFC
};

#define PCI_REGISTER(reg)  ((reg)&0xFC)

enum {
    /* bits order in CONFIG_ADDRESS */
    REGISTER = 2,
    FUNCTION = 8,
    DEVICE   = 11,
    BUS      = 16,
    RESERVED = 24,
    ENABLE   = 31
};

enum {
    VENDOR_ID   = 0x00,
    DEVICE_ID   = 0x02,
    COMMAND     = 0x04,
    STATUS      = 0x06,
    REVISION_ID = 0x08,
    PROG_IF     = 0x09,
    SUBCLASS    = 0x0A,
    CLASS_CODE  = 0x0B,
    CACHE_LINE_SIZE = 0x0C,
    LATENCY_TIMER = 0x0D,
    HEADER_TYPE = 0x0E,
    BIST        = 0x0F,
    BAR0        = 0x10,
    BAR1        = 0x14,
    BAR2        = 0x18,
    BAR3        = 0x1C,
    BAR4        = 0x20,
    BAR5        = 0x24,
    CARDBUS_CIS_POINTER = 0x28,
    SUBSYSTEM_VENDOR_ID = 0x2C,
    SUBSYSTEM_ID = 0x2E,
    EXPANSION_ROM_BASE_ADDRESS = 0x30,
    CAPABILITES_POINTER = 0x34,
    INTERRUPT_LINE  = 0x3C,
    INTERRUPT_PIN   = 0x3D,
    MIN_GRANT   = 0x3E,
    MAX_LATENCY = 0x3F
};

enum {
    PCI_UNKNOWN = 0xFFFFFFFFUL
};

uint8_t pci_read_8(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg);
uint16_t pci_read_16(uint32_t bus, uint32_t device, uint32_t func, uint32_t 
		reg);
uint32_t pci_read_32(uint32_t bus, uint32_t device, uint32_t func, uint32_t 
        reg);

void bus_pci_init(void);

#endif
#endif

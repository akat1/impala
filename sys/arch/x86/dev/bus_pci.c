/* Impala Operating System
 *
 * Copyright (C) 2010 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
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

/*
 * Kod narazie bedzie brzydki, pisze tak, zeby cos zadzialalo, a pozniej go
 * porozkladam logicznie. KMWTW. Wspieramy tylko intel-compatibile
 *                                  - shm
 * 
 * Zainteresowanych odsy�am do:
 * http://wiki.osdev.org/PCI
 * http://fxr.watson.org/fxr/source/drivers/libpci/?v=minix-3-1-1
 * http://www.acm.uiuc.edu/sigops/roll_your_own/7.c.0.html
 *
 * TODO:
 * - dodac liste urzadzen (mozna takie bez problemu znalezc w internecie, mozna
 * tez zabrac z OpenBSD)
 *
 */ 

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/utils.h>
#include <machine/types.h>
#include <machine/io.h>
#include <machine/bus/pci.h>
#include <machine/bus/pci_devices.h>

uint32_t _pci_config_address(uint32_t bus, uint32_t device, uint32_t func, 
        uint32_t reg);

uint32_t _pci_read(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg);


uint16_t _pci_read_16(uint32_t bus, uint32_t device, uint32_t func, uint32_t 
        reg);
uint8_t _pci_read_8(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg);
uint32_t _pci_read_32(uint32_t bus, uint32_t device, uint32_t func, uint32_t 
        reg);

enum {
    CONFIG_ADDRES   = 0xCF8,
    CONFIG_DATA     = 0xCFC
};

enum {
    /* ulozenie bitow w rejestrze config address */
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

uint32_t
_pci_config_address(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    // cleanup
    uint32_t x;
    x = ( 1 << ENABLE | (bus<<BUS) | (device<<DEVICE) | (func<<FUNCTION) | 
            ( reg << REGISTER ) );
//:    kprintf("a: %x\n", x);
    return x;
}

/* @bug: Tutaj trzeba jakas lepsza nazwe wymyslec :[ a reszte funkcji przeobic
 * na makra */
uint32_t
_pci_read(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    io_out32(CONFIG_ADDRES, _pci_config_address(bus, device, func, reg));
    return io_in32(CONFIG_DATA);
}

#define PCI_REGISTER(reg)  ((reg)&0xFC)

uint8_t
_pci_read_8(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    uint32_t ret;
    
    ret = _pci_read(bus, device, func, PCI_REGISTER(reg));
    ret &= 0xFFUL << (reg-PCI_REGISTER(reg))*8;
    ret >>= (reg-PCI_REGISTER(reg))*8;
    return (uint8_t)ret;
}

uint16_t
_pci_read_16(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    uint32_t ret;
    
    ret = _pci_read(bus, device, func, PCI_REGISTER(reg));
    ret &= 0xFFFFUL << (reg-PCI_REGISTER(reg))*8;
    ret >>= (reg-PCI_REGISTER(reg))*8;
    return (uint16_t)ret;
}

uint32_t
_pci_read_32(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    return _pci_read(bus, device, func, PCI_REGISTER(reg));
}

struct pci_device_info *
pci_device_info(uint16_t vendor_id, uint16_t device_id);

struct pci_device_info *
pci_device_info(uint16_t vendor_id, uint16_t device_id)
{
    int i;

    for ( i = 0 ;; i++ ) {
        if ( (pci_devices_table[i].vendor_id == vendor_id 
                && pci_devices_table[i].device_id == device_id) 
             || pci_devices_table[i].vendor_id == (uint16_t)PCI_UNKNOWN
            )
            return &(pci_devices_table[i]);
    }

    /* NOT REACHABLE */
    KASSERT(0);
    return NULL;
}

struct pci_vendor_info *
pci_vendor_info(uint16_t vendor_id);
struct pci_vendor_info *
pci_vendor_info(uint16_t vendor_id)
{
    int i;

    for ( i = 0 ;; i++ ) {
        if ( pci_vendors_table[i].vendor_id == vendor_id
                || pci_vendors_table[i].vendor_id == (uint16_t)PCI_UNKNOWN )
            return &(pci_vendors_table[i]);
    }

    /* NOT REACHABLE */
    KASSERT(0);
    return NULL;
};

bool _pci_probe_device(uint8_t bus, uint8_t device, uint8_t func);

bool
_pci_probe_device(uint8_t bus, uint8_t device, uint8_t func)
{
    return ( _pci_read_16(bus, device, func, VENDOR_ID) !=
            (uint16_t)PCI_UNKNOWN ) ? TRUE : FALSE;
}

bool _pci_attach_device(uint8_t bus, uint8_t device, uint8_t func);

bool
_pci_attach_device(uint8_t bus, uint8_t device, uint8_t func)
{
    if ( ! _pci_probe_device(bus, device, func) )
        return FALSE;


    pci_dev[pci_dev_nr].bus = bus;
    pci_dev[pci_dev_nr].device = device;
    pci_dev[pci_dev_nr].func = func;
    pci_dev[pci_dev_nr].vendor_id = 
        _pci_read_16(bus, device, func, VENDOR_ID);
    pci_dev[pci_dev_nr].device_id = 
        _pci_read_16(bus, device, func, DEVICE_ID);
    pci_dev[pci_dev_nr].device_info = 
        pci_device_info(pci_dev[pci_dev_nr].vendor_id,  
                pci_dev[pci_dev_nr].device_id);

    pci_dev[pci_dev_nr].vendor_info =
        pci_vendor_info(pci_dev[pci_dev_nr].vendor_id);

    pci_dev_nr++;

    return TRUE;
}

void
bus_pci_init()
{
    uint16_t device;
    uint16_t vendor;
    int bus, dev, func;

    bus = 4;
    device = _pci_read_16(0,0,0,DEVICE_ID);
    vendor = _pci_read_16(0,0,0,VENDOR_ID);

    if ( device == (uint16_t)PCI_UNKNOWN && vendor == (uint16_t)PCI_UNKNOWN )
        /* Nie znalezlismy kontrolera PCI */
        return;

    kprintf("PCI controller: %s\n", 
            (pci_device_info(vendor,device))->device_name);


    // @bug, BUS==0?
    for ( dev = 0 ; dev < 32 ; dev++ ) {
        for ( func = 0 ; func < 8 ; func++ ) {
            if ( _pci_probe_device(0, dev, func) ) {
                _pci_attach_device(0, dev, func);
            }
        }
    }

    for ( dev = 0 ; dev < pci_dev_nr ; dev++ ) {
        kprintf("v: %x d: %x i: %s\n", pci_dev[dev].vendor_id,
                pci_dev[dev].device_id, pci_dev[dev].device_info->device_name);
    }

//    for(;;);

    return;
}

/*
shm's editor
vim:tw=80:expandtab:fileencoding=iso-8859-2
*/

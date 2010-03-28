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

/*
 * Tested on QEMU, for now we're supporting only INTEL controller. KMDzTDz (Komu
 * ma dzialac temu dziala).
 *                                  - shm
 * 
 * @TODO:
 * - memory allocation for both PCI memory and PCI I/O
 * - fix PCI interface
 * - add more controllers
 * - support more devices
 * - implement pci-pci bridge support (as i understand PCI for now QEMU don't
 *   need it), it's needed to run on real machine
 * 
 * Following resources was used:
 * http://wiki.osdev.org/PCI - briefly PCI descritpion
 * http://tldp.org/LDP/tlk/dd/pci.html - description how PCI is handled in Linux
 * http://fxr.watson.org/fxr/source/drivers/libpci/?v=minix-3-1-1 - minix PCI
 * driver
 * http://fxr.watson.org/fxr/source/pc/pci.c?v=PLAN9 - Plan9 PCI driver
 * http://www.acm.uiuc.edu/sigops/roll_your_own/7.c.0.html - useful tables
 */ 

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/utils.h>
#include <machine/types.h>
#include <machine/io.h>
#include <machine/bus/pci.h>
#include <machine/bus/pci_devices.h>

uint32_t _pci_config_address(uint32_t bus, uint32_t device,
        uint32_t func, uint32_t reg);
uint32_t _pci_read(uint32_t bus, uint32_t device, 
        uint32_t func, uint32_t reg);
bool _pci_register_device(uint8_t bus, uint8_t device, uint8_t func);
struct pci_vendor_info *pci_vendor_info(uint16_t vendor_id);
bool _pci_probe_device(uint8_t bus, uint8_t device, uint8_t func);
struct pci_device_info * pci_device_info(uint16_t vendor_id, 
        uint16_t device_id);
struct pci_baseclass_info * pci_baseclass_info(uint8_t baseclass);

uint32_t
_pci_config_address(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    /* @bug: add bit masks for each piece */
    return ( 1 << ENABLE | 
            (bus<<BUS) | 
            (device<<DEVICE) | 
            (func<<FUNCTION) | 
            ( (reg>>2) << REGISTER ) );
}

uint32_t
_pci_read(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    io_out32(CONFIG_ADDRES, _pci_config_address(bus, device, func, reg));
    return io_in32(CONFIG_DATA);
}


uint8_t
pci_read_8(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    uint32_t ret;
    
    ret = _pci_read(bus, device, func, PCI_REGISTER(reg));
    ret &= 0xFFUL << (reg-PCI_REGISTER(reg))*8;
    ret >>= (reg-PCI_REGISTER(reg))*8;
    return (uint8_t)ret;
}

uint16_t
pci_read_16(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    uint32_t ret;
    
    ret = _pci_read(bus, device, func, PCI_REGISTER(reg));
    ret &= 0xFFFFUL << (reg-PCI_REGISTER(reg))*8;
    ret >>= (reg-PCI_REGISTER(reg))*8;
    return (uint16_t)ret;
}

uint32_t
pci_read_32(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    return _pci_read(bus, device, func, PCI_REGISTER(reg));
}



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
    panic("NOT REACHABLE");
    return NULL;
}

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
    panic("NOT REACHABLE");
    return NULL;
};

struct pci_baseclass_info *
pci_baseclass_info(uint8_t baseclass)
{
    int i;

    for ( i = 0 ;; i++ ) {
        if ( pci_baseclasses[i].baseclass == baseclass )
            return &pci_baseclasses[i];
        if ( pci_baseclasses[i].baseclass == 0xFF )
            panic("PCI device unknown baseclass: %x!", baseclass);
    }

    /* NOT REACHED */
    panic("NOT REACHED");
    return NULL;
}

bool
_pci_probe_device(uint8_t bus, uint8_t device, uint8_t func)
{
    return ( pci_read_16(bus, device, func, VENDOR_ID) !=
            (uint16_t)PCI_UNKNOWN ) ? TRUE : FALSE;
}


bool
_pci_register_device(uint8_t bus, uint8_t device, uint8_t func)
{
    if ( ! _pci_probe_device(bus, device, func) )
        return FALSE;


    if ( pci_dev_nr >= MAX_PCI_DEV )
        panic("Too many PCI devices...");

    /* register device in pci_dev table */
    pci_dev[pci_dev_nr].bus = bus;
    pci_dev[pci_dev_nr].device = device;
    pci_dev[pci_dev_nr].func = func;
    pci_dev[pci_dev_nr].vendor_id = 
        pci_read_16(bus, device, func, VENDOR_ID);
    pci_dev[pci_dev_nr].device_id =  
        pci_read_16(bus, device, func, DEVICE_ID);
    pci_dev[pci_dev_nr].baseclass =
        pci_read_8(bus, device, func, CLASS_CODE);
    pci_dev[pci_dev_nr].device_info = 
        pci_device_info(pci_dev[pci_dev_nr].vendor_id,  
                pci_dev[pci_dev_nr].device_id);
    pci_dev[pci_dev_nr].vendor_info =
        pci_vendor_info(pci_dev[pci_dev_nr].vendor_id);
    pci_dev[pci_dev_nr].baseclass_info =
        pci_baseclass_info(pci_dev[pci_dev_nr].baseclass);

    pci_dev_nr++;

    return TRUE;
}

void
bus_pci_init()
{
    int bus, dev, func, i;

    if ( ! _pci_probe_device(0,0,0) )
        /* No PCI controller found, exiting */
        return;

    // @bug, BUS==0?
    // Register all devices
    for ( bus = 0 ; bus < 256 ; bus++ ) {
        for ( dev = 0 ; dev < 32 ; dev++ ) {
            for ( func = 0 ; func < 8 ; func++ ) {
                if ( _pci_probe_device(bus, dev, func) )
                    _pci_register_device(bus, dev, func);
            }
        }
    }

    kprintf("%x\n", pci_dev_nr);

    for ( dev = 0 ; dev < pci_dev_nr ; dev++ ) {
        kprintf("v: %x d: %x i: %s bdf: %x:%x:%x c: %s\n", pci_dev[dev].vendor_id,
                pci_dev[dev].device_id, pci_dev[dev].device_info->device_name,
                pci_dev[dev].bus, pci_dev[dev].device, pci_dev[dev].func,
                pci_dev[dev].baseclass_info->baseclass_name);
        /* PRINT BARS */
        for ( i = 0 ; i < 6 ; i++ )
        {
            kprintf("%x|",pci_read_32(pci_dev[dev].bus,
                        pci_dev[dev].device, pci_dev[dev].func, BAR0+i*4));
        }
        kprintf("\n");
    }
   
    // @bug: handle irq
    // ...
    for(;;);

    return;
}

/*
shm's editor
vim:tw=80:expandtab
*/

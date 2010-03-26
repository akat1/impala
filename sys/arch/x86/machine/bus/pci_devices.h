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

#include <sys/types.h>

#ifndef __MACHINE_BUS_PCI_DEVICES_H
#define __MACHINE_BUS_PCI_DEVICES_H
#ifdef __KERNEL

/*
   Moze byc uzyteczne:
   @bug: posprzataj mnie
   http://fxr.watson.org/fxr/source/drivers/libpci/pci_table.c?v=minix-3-1-1
*/

#define MAX_PCI_DEV		20

struct pci_controller {
	struct pci_device_info *device_info;
	uint8_t bus;
};

struct pci_device {
	struct pci_device_info *device_info;
	struct pci_vendor_info *vendor_info;
	struct pci_subclass *baseclass;
	/* dodac sub/infclass */
	uint8_t	bus;
	uint8_t device;
	uint8_t func;
	uint16_t vendor_id;
	uint16_t device_id;
} pci_dev[MAX_PCI_DEV];

int pci_dev_nr = 0;

struct pci_device_info {
	uint16_t	vendor_id;
	uint16_t	device_id;
	char*		device_name;
};

struct pci_vendor_info {
	uint16_t	vendor_id;
	char*		vendor_name;
};

struct pci_baseclass {
	uint8_t		baseclass;
	char		*baseclass_name;
};

struct pci_device_info pci_devices_table[] =
{
	{0x8086, 0x1237, "Intel 82441FX (440FX)"},
	{0x8086, 0x7000, "Intel 82371SB"},
	{0x8086, 0x7010, "Intel 82371SB (IDE)"},
	{0x8086, 0x7113, "Intel 82371AB (Power)"},
	{0x1013, 0x00B8, "Cirrus Logic GD 5446"},
	{0x10EC, 0x8139, "Realtek RTL8139" },
	{0x0000, 0x0000, "Unknown device"}
};

/* Producenci */
struct pci_vendor_info pci_vendors_table[] =
{
	{0x8086, "Intel"},
	{0x1013, "Cirrus Logic" },
	{0x10EC, "Realtek" },
	{0x0000, "Unknown vendor"}
};

/* Baseclass - bazujacy na:
 * http://www.acm.uiuc.edu/sigops/roll_your_own/7.c.1.html
 */
struct pci_baseclass pci_baseclasses[] =
{
	{0x00, "No class"},
	{0x01, "Mass storage"},
	{0x02, "Network controller"},
	{0x03, "Display controller"},
	{0x04, "Multimedia device"},
	{0x05, "Memory Controller"},
	{0x06, "Bridge Device"},
	{0x07, "Simple communications controllers"},
	{0x08, "Base system peripherals"},
	{0x09, "Inupt devices"},
	{0x0A, "Docking Stations"},
	{0x0B, "Processor"},
	{0x0C, "Serial bus controllers"},
	{0xFF, "Misc device"}
};
#endif
#endif

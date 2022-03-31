/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#include <sys/types.h>

#ifndef __MACHINE_BUS_PCI_DEVICES_H
#define __MACHINE_BUS_PCI_DEVICES_H
#ifdef __KERNEL

/*
 * Mostly based on MINIX pci tables:
 * http://fxr.watson.org/fxr/source/drivers/libpci/pci_table.c?v=minix-3-1-1
 */

#define MAX_PCI_DEV		20

struct pci_controller {
	struct pci_device_info *device_info;
	uint8_t bus;
};

struct pci_device {
	struct      pci_device_info *device_info;
	struct      pci_vendor_info *vendor_info;
	struct      pci_baseclass_info *baseclass_info;
	/* add sub/infclass */
	uint8_t     bus;
	uint8_t     device;
	uint8_t     func;
	uint8_t     baseclass;
	uint16_t    vendor_id;
	uint16_t    device_id;
    /* interrupt */
};

extern struct pci_device pci_dev[MAX_PCI_DEV];
extern int pci_dev_nr;

struct pci_device_info {
	uint16_t	vendor_id;
	uint16_t	device_id;
	char        *device_name;
};

struct pci_vendor_info {
	uint16_t	vendor_id;
	char        *vendor_name;
};

struct pci_baseclass_info {
	uint8_t		baseclass;
	char		*baseclass_name;
};

extern struct pci_device_info pci_devices_table[];
extern struct pci_vendor_info pci_vendors_table[];
extern struct pci_baseclass_info pci_baseclasses[];
#endif
#endif

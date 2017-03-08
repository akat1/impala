/* XXX: relicense me */

#ifndef __MACHINE_BUS_PCI_H
#define __MACHINE_BUS_PCI_H
#ifdef __KERNEL

#include <machine/bus/pci_devices.h>

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


uint8_t pci_dev_read_8(struct pci_device *dev, uint32_t reg);
uint16_t pci_dev_read_16(struct pci_device *dev, uint32_t reg);
uint32_t pci_dev_read_32(struct pci_device *dev, uint32_t reg);

/*
void pci_map_range_io(dev, vaddr, size_t)
void pci_map_range_mem(dev, vaddr, size_t)
void pci_intr_register(...)
*/

/* XXX: interrupts */
/* XXX: maps */

void bus_pci_init(void);

#endif
#endif

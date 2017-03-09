/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

/*
 * Tested on QEMU, for now we're supporting only INTEL controller. 
 * KMDzTDz (Komu ma dzialac temu dziala).
 *                                  - shm
 * 
 * @TODO:
 *
 * - memory allocation for both PCI memory and PCI I/O (?)
 * - fix PCI interface
 * - implement pci-pci bridge support (as I understand PCI for now QEMU don't
 *   need it), it's needed to run on a real machine
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

int pci_dev_nr = 0;
struct pci_device pci_dev[MAX_PCI_DEV];

/* XXX:
 * Only qemu hardware is listed here, feel free to extend this list.
 *                                     -shm
 */

struct pci_device_info pci_devices_table[] =
{
	{0x8086, 0x1237, "Intel 82441FX (440FX)"},
	{0x8086, 0x7000, "Intel 82371SB (ISA)"},
	{0x8086, 0x7010, "Intel 82371SB (IDE)"},
	{0x8086, 0x7113, "Intel 82371AB (Power)"},
    {0x8086, 0x100e, "Intel 82540EM Gigabit Ethernet Controller "},
	{0x1013, 0x00B8, "Cirrus Logic GD 5446"},
	{0x10EC, 0x8139, "Realtek RTL8139" },
    {0x1234, 0x1111, "VGA compatible controller"},
	{0xFFFF, 0xFFFF, "Unknown device"}
};

struct pci_vendor_info pci_vendors_table[] =
{
	{0x8086, "Intel"},
	{0x1013, "Cirrus Logic" },
	{0x10EC, "Realtek" },
	{0xFFFF, "Unknown vendor"}
};

/* Baseclass based on:
 * http://www.acm.uiuc.edu/sigops/roll_your_own/7.c.1.html
 */
struct pci_baseclass_info pci_baseclasses[] =
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

static void _pci_write(uint32_t bus, uint32_t device, uint32_t func, uint32_t
  reg, uint32_t val);

uint8_t pci_read_8(uint32_t bus, uint32_t device, uint32_t func, uint32_t
  reg);
uint16_t pci_read_16(uint32_t bus, uint32_t device, uint32_t func, uint32_t
  reg);
uint32_t pci_read_32(uint32_t bus, uint32_t device, uint32_t func, uint32_t 
  reg);
void _pci_print_devices(void);
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
static void _pci_scan_devices(void);

uint32_t
_pci_config_address(uint32_t bus, uint32_t device, uint32_t func,
  uint32_t reg)
{
    return ( 1 << ENABLE | 
            (bus<<BUS) | 
            (device<<DEVICE) | 
            (func<<FUNCTION) | 
            ( (reg>>2) << REGISTER ) );
}

/*****************************************************************************
 * 
 * WRITE
 *
 */

static void
_pci_write(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg,
  uint32_t val)
{
    io_out32(CONFIG_ADDRESS, _pci_config_address(bus, device, func, reg));
    io_out32(CONFIG_DATA, val);
}

void
pci_dev_write_32(struct pci_device *dev, uint32_t reg, uint32_t val)
{
    _pci_write(dev->bus, dev->device, dev->func, reg, val);
}

/*****************************************************************************
 * 
 * READ
 *
 */

uint32_t
_pci_read(uint32_t bus, uint32_t device, uint32_t func, uint32_t reg)
{
    io_out32(CONFIG_ADDRESS, _pci_config_address(bus, device, func, reg));
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

uint8_t
pci_dev_read_8(struct pci_device *dev, uint32_t reg)
{
    return pci_read_8(dev->bus, dev->device, dev->func, reg);
}

uint16_t
pci_dev_read_16(struct pci_device *dev, uint32_t reg)
{
    return pci_read_16(dev->bus, dev->device, dev->func, reg);
}

uint32_t
pci_dev_read_32(struct pci_device *dev, uint32_t reg)
{
    return pci_read_32(dev->bus, dev->device, dev->func, reg);
}

/*****************************************************************************
 * 
 * HELPERS
 *
 */

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

    /* XXX: What the heck? */
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

    /* initialize driver */

    pci_dev_nr++;

    return TRUE;
}

static void
_pci_scan_devices()
{
    int bus, dev, func;

    /* Probe all devices */
    for ( bus = 0 ; bus < 256 ; bus++ ) {
        for ( dev = 0 ; dev < 32 ; dev++ ) {
            for ( func = 0 ; func < 8 ; func++ ) {
                if ( _pci_probe_device(bus, dev, func) )
                    _pci_register_device(bus, dev, func);
            }
        }
    }
}

void _pci_print_devices()
{
    int dev, i;

    /* Print What we have */
    for ( dev = 0 ; dev < pci_dev_nr ; dev++ ) {
        /* GENERAL INFO */
        kprintf("pci-probe: %x d: %x i: %s bdf: %x:%x:%x c: %s\n",
                pci_dev[dev].vendor_id, pci_dev[dev].device_id,
                pci_dev[dev].device_info->device_name,
                pci_dev[dev].bus, pci_dev[dev].device, pci_dev[dev].func,
                pci_dev[dev].baseclass_info->baseclass_name);

        /* PRINT BARS */
        for ( i = 0 ; i < 6 ; i++ )
            kprintf("%x|",pci_dev_read_32(&pci_dev[dev], BAR0+i*4));
        kprintf("\n");
    }
}

void
bus_pci_init()
{

    if ( ! _pci_probe_device(0,0,0) ) {
        /* No PCI controller found, exiting */
        return;
    }

    _pci_scan_devices();
    _pci_print_devices();
   
    // @bug: handle irq
    // ...
    // for(;;);
    return;
}

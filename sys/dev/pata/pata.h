/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#ifndef __DEV_PATA_PATA_H
#define __DEV_PATA_PATA_H
#ifdef __KERNEL

void pata_init(void);

/* XXX:
 * screw CHS
 * LBA28
 * LBA48
 */

/* 
 * http://ethv.net/workshops/osdev/notes/notes-5 
 * http://wiki.osdev.org/ATA_PIO_Mode#Registers
 */

#define PATA_REGISTER(X, Y) ((X) + (Y))

enum {
    /* I/O ports */
    PATA_PRIMARY        = 0x1F0,
    PATA_SECONDARY      = 0x170,
    /* Device Control Register / Alternate Status */
    PATA_PRIMARY_DCR_AS     = 0x3F6,
    PATA_SECONDARY_DCR_AS   = 0x376
};

enum {
    PATA_REG_DATA       = 0x00,
    PATA_REG_FEATURES   = 0x01,
    PATA_REG_ERROR      = 0x01,
    PATA_REG_SECTOR_CNT = 0x02,
    PATA_REG_LBA_LO     = 0x03,
    PATA_REG_LBA_MID    = 0x04,
    PATA_REG_LBA_HI     = 0x05,
    PATA_REG_DRIVE      = 0x06,
    PATA_REG_COMMAND    = 0x07,
    PATA_REG_STATUS     = 0x07
};

enum {
    PATA_STATUS_ERR     = 0,
    PATA_STATUS_DRQ     = (1<<3),
    PATA_STATUS_SRV     = (1<<4),
    PATA_STATUS_DF      = (1<<5),
    PATA_STATUS_RDY     = (1<<6),
    PATA_STATUS_BSY     = (1<<7)
};

enum {
    PATA_IDENTIFY_DRIVE     = 0,
    PATA_IDENTIFY_FEATURES  = 83
};

enum {
    PATA_CTRL_NIEN  = (1<<1),
    PATA_CTRL_SRST  = (2<<1),
    PATA_CTRL_HOB   = (7<<1)
};

enum {
    PATA_CMD_PIO_READ           = 0x20,
    PATA_CMD_PIO_WRITE          = 0x30,
	PATA_CMD_READ_DMA_LBA28     = 0xC8,
	PATA_CMD_READ_DMA_LBA48     = 0x25,
	PATA_CMD_WRITE_DMA_LBA28    = 0xCA,
	PATA_CMD_WRITE_DMA_LBA48    = 0x35,
    PATA_CMD_CACHE_FLUSH        = 0xE7,
    PATA_CMD_IDENTIFY           = 0xEC
};

enum {
    PATA_MASTER = 0xA0,
    PATA_SLAVE  = 0xB0
};

enum {
    IDE_PATA    = 0x00,
    IDE_ATAPI   = 0x01
};
#endif
#endif

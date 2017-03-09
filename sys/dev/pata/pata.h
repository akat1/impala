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

#ifndef __DEV_PATA_PATA_H
#define __DEV_PATA_PATA_H
#ifdef __KERNEL

void pata_init(void);

/* XXX:
   screw CHS
   LBA28
   LBA48
*/

/* 
 * http://ethv.net/workshops/osdev/notes/notes-5 
 * http://wiki.osdev.org/ATA_PIO_Mode#Registers
 */

#define PATA_REGISTER(X, Y) (X + Y)

enum {
    /* IO ports */
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

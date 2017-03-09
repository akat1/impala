/* Impala Operating System
 *
 * Copyright (C) 2010 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2010 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
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

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/bio.h>
#include <dev/pata/pata.h>
#include <fs/devfs/devfs.h>
#include <machine/interrupt.h>
#include <machine/io.h>
#include <machine/bus/pci.h>

/* For now we implement only PIO mode.
 * The good news is that every ATA device must implement it (in order to
 * confirm the standard)
 * XXX: The bad news is it's horrible slow, who cares for now!
 *
 * TODO:
 *
 * - major clean up
 * - add write
 * - physio
 * - get rid of PIO
 */

/*
 * I'm not sure if pata is right name.
 * ...
 * Reading material:
 * http://suif.stanford.edu/~csapuntz/specs/idems100.ps
 */

/* no more devices than 4: 2 x PRIMARY + 2 x SECONDARY */
#define PATA_DEVICES_MAX    (4)

struct pata_device {
    uint16_t bus;   /* PRIMARY or SECONDARY */
    uint8_t disk;  /* MASTER or SLAVE */
    const char *name;
    devd_t  *dev;   /* Associated device */
    uint8_t busy;   /* am i busy? */
    /* ... */
} pata_devices[PATA_DEVICES_MAX];

int pata_devices_no = 0;

/* private */
void _pata_set_drive(int bus, int disk);
void _pata_send_command(int bus, int cmd);
uint8_t _pata_read_status(int bus);
void _pata_probe(int bus, int disk);
void _pata_register(int bus, int disk, uint16_t *identify);
size_t _pata_read_lba28(struct pata_device *dev, uint16_t *buf, uint32_t addr,
  uint8_t count);
int pata_open(devd_t *d, int flags);
int pata_close(devd_t *d);
int pata_strategy(devd_t *d, iobuf_t *bp);

devsw_t pata_devsw = {
    pata_open,
    pata_close,
    noioctl,
    noread,
    nowrite,
    pata_strategy,
    DEV_BDEV
};


/*
 * Sets current disk on particular bus
 * @bus - primary/secondary
 * @disk - master/slave
 */
void
_pata_set_drive(int bus, int disk)
{
    io_out8(PATA_REGISTER(bus, PATA_REG_DRIVE), disk);
}

/*
 _pata_probe
 _pata_register
*/

void
_pata_send_command(int bus, int cmd)
{
    io_out8(PATA_REGISTER(bus, PATA_REG_COMMAND), cmd);
}


uint8_t
_pata_read_status(int bus)
{
    return io_in8(PATA_REGISTER(bus, PATA_REG_STATUS));
}


/* read sector */
size_t
_pata_read_lba28(struct pata_device *dev, uint16_t *buf, uint32_t addr,
        uint8_t count)
{
    uint8_t drv, status, sect;
    int i;

    /* XXX: move it to set_drive */
    drv = (dev->disk == PATA_MASTER) ? 0xE0 : 0xF0;
    drv |= (addr >> 24) & 0x0F;

    io_out8(PATA_REGISTER(dev->bus, PATA_REG_DRIVE), drv);
    io_out8(PATA_REGISTER(dev->bus, PATA_REG_FEATURES), 0);
    io_out8(PATA_REGISTER(dev->bus, PATA_REG_SECTOR_CNT), count);

    /* lba28 */
    io_out8(PATA_REGISTER(dev->bus, PATA_REG_LBA_LO), addr & 0xFF);
    io_out8(PATA_REGISTER(dev->bus, PATA_REG_LBA_MID), (addr >> 8) & 0xFF);
    io_out8(PATA_REGISTER(dev->bus, PATA_REG_LBA_HI), (addr >> 16) & 0xFF);

    _pata_send_command(dev->bus, PATA_CMD_PIO_READ);

    for (sect = 0; sect < count; sect++) {
        /* 400ns delay (reading status takes 100ns) */
        for (i = 0; i < 4; i++) {
            (void)_pata_read_status(dev->bus);
        }

        /* XXX: irq some day */
        for(;;) {
            status = _pata_read_status(dev->bus);
            if ((status & PATA_STATUS_ERR) != 0)
                return -1;
            if ((status & PATA_STATUS_DF) != 0)
                return -1;
            if ((status & PATA_STATUS_BSY) == 0 &&
             (status & PATA_STATUS_DRQ) != 0)
                break;
        }


        for (i = 0; i < 256; i++) {
            /* XXX: repinsw? */
            buf[sect*256+i] = io_in16(PATA_REGISTER(dev->bus, PATA_REG_DATA));
        }
    }

    return count;
}

void
_pata_register(int bus, int disk, uint16_t *identify)
{
    pata_devices[pata_devices_no].bus = bus;
    pata_devices[pata_devices_no].disk = disk;

    /* XXX: deuglify me */
    if (bus == PATA_PRIMARY) {
        if (disk == PATA_MASTER)
            pata_devices[pata_devices_no].name = "ata0d0";
        else
            pata_devices[pata_devices_no].name = "ata0d1";
    } else {
        if (disk == PATA_MASTER)
            pata_devices[pata_devices_no].name = "ata1d0";
        else
            pata_devices[pata_devices_no].name = "ata1d1";
    }

    pata_devices[pata_devices_no].dev = devd_create(&pata_devsw,
     pata_devices[pata_devices_no].name, -1, &pata_devices[pata_devices_no]);
    devfs_register(pata_devices[pata_devices_no].dev, 0, 0, 0600);

    pata_devices_no++;
}

void
_pata_probe(int bus, int disk)
{
    int i;
    uint16_t identify[256];

    /* select drive */
    _pata_set_drive(bus, disk);

    /* set 0s */
    io_out8(PATA_REGISTER(bus, PATA_REG_SECTOR_CNT), 0);
    io_out8(PATA_REGISTER(bus, PATA_REG_LBA_LO), 0);
    io_out8(PATA_REGISTER(bus, PATA_REG_LBA_MID), 0);
    io_out8(PATA_REGISTER(bus, PATA_REG_LBA_HI), 0);

    /* send IDENTIFY */
    _pata_send_command(bus, PATA_CMD_IDENTIFY);

    /* read status */
    if (_pata_read_status(bus) == 0) {
        /* drive does not exists */
        return;
    }

    /* wait until ready */
    while (_pata_read_status(bus) & PATA_STATUS_BSY);

    /* could be ATAPI drive, according to:
     * http://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command */
    if (io_in8(PATA_REGISTER(bus, PATA_REG_LBA_MID)) != 0 ||
      io_in8(PATA_REGISTER(bus, PATA_REG_LBA_HI)) != 0) {
        /* found ATAPI device, run away */
        return;
    }

    /* wait until data is ready */
    while ((_pata_read_status(bus) & (PATA_STATUS_DRQ | PATA_STATUS_ERR))
      == 0);

    if (_pata_read_status(bus) & PATA_STATUS_ERR) {
        /* error has occured, screw that drive */
        return;
    }

    /* read data */
    for (i = 0 ; i < 256; i++)
        identify[i] = io_in16(PATA_REGISTER(bus, PATA_REG_DATA));

    /* register device */
    _pata_register(bus, disk, identify);
}

void
pata_init(void)
{
    _pata_probe(PATA_PRIMARY, PATA_MASTER);
    _pata_probe(PATA_PRIMARY, PATA_SLAVE);
    _pata_probe(PATA_SECONDARY, PATA_MASTER);
    _pata_probe(PATA_SECONDARY, PATA_SLAVE);

}

int
pata_open(devd_t *d, int flags)
{
    struct pata_device *dev = d->priv;
    if (dev->busy != FALSE) {
        return -EBUSY;
    }
    dev->busy = TRUE;
    return 0;
}

int
pata_close(devd_t *d)
{
    struct pata_device *dev = d->priv;
    dev->busy = FALSE;
    return 0;
}

int
pata_strategy(devd_t *d, iobuf_t *b)
{
    int s;

    struct pata_device *dev = d->priv;
    s = splbio();
    if (b->oper) {
        /* XXX: bcount is bounded here */
        _pata_read_lba28(dev, b->addr, b->blkno, b->bcount);
    } else
        /* not supported for now */
        b->flags |= BIO_ERROR;
    splx(s);
    bio_done(b);
    return 0;
}

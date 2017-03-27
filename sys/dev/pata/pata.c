/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
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
 * - lba48 support
 * - physio
 * - get rid of PIO
 *
 * Reading material:
 * http://suif.stanford.edu/~csapuntz/specs/idems100.ps
 * http://www.fysnet.net/media_storage_devices.htm
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
static void _pata_set_drive(int bus, int disk);
static void _pata_send_command(int bus, int cmd);
static uint8_t _pata_read_status(int bus);
static void _pata_probe(int bus, int disk);
static void _pata_register(int bus, int disk, uint16_t *identify);
static size_t _pata_read_lba28(struct pata_device *dev, uint16_t *buf,
  uint32_t addr, uint8_t count);
static int pata_open(devd_t *d, int flags);
static int pata_close(devd_t *d);
static int pata_strategy(devd_t *d, iobuf_t *bp);

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
static void
_pata_set_drive(int bus, int disk)
{
    io_out8(PATA_REGISTER(bus, PATA_REG_DRIVE), disk);
}

static void
_pata_send_command(int bus, int cmd)
{
    io_out8(PATA_REGISTER(bus, PATA_REG_COMMAND), cmd);
}


static uint8_t
_pata_read_status(int bus)
{
    return io_in8(PATA_REGISTER(bus, PATA_REG_STATUS));
}


/* read sectors */
static size_t
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
            if (ISSET(status, PATA_STATUS_ERR))
                return -1;
            if (ISSET(status, PATA_STATUS_DF))
                return -1;
            if (ISUNSET(status, PATA_STATUS_BSY) &&
              ISSET(status, PATA_STATUS_DRQ))
                break;
        }

        for (i = 0; i < 256; i++) {
            /* XXX: repinsw? */
            buf[sect*256+i] = io_in16(PATA_REGISTER(dev->bus, PATA_REG_DATA));
        }
    }

    return count;
}

/* write sectors */
static size_t
_pata_write_lba28(struct pata_device *dev, uint16_t *buf, uint32_t addr,
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

    _pata_send_command(dev->bus, PATA_CMD_PIO_WRITE);

    for (sect = 0; sect < count; sect++) {
        /* 400ns delay (reading status takes 100ns) */
        for (i = 0; i < 4; i++) {
            (void)_pata_read_status(dev->bus);
        }

        /* XXX: irq some day */
        for(;;) {
            status = _pata_read_status(dev->bus);
            if (ISSET(status, PATA_STATUS_ERR))
                return -1;
            if (ISSET(status, PATA_STATUS_DF))
                return -1;
            if (ISUNSET(status, PATA_STATUS_BSY) &&
              ISSET(status, PATA_STATUS_DRQ))
                break;
        }

        for (i = 0; i < 256; i++) {
            io_out16(PATA_REGISTER(dev->bus, PATA_REG_DATA), buf[sect*256+i]);
        }

        /* XXX: is it really necessary? */
        _pata_send_command(dev->bus, PATA_CMD_CACHE_FLUSH);
    }


    return count;
}

static void
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

static void
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
    while (ISSET(_pata_read_status(bus), PATA_STATUS_BSY));

    /* could be ATAPI drive, according to:
     * http://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command */
    if (io_in8(PATA_REGISTER(bus, PATA_REG_LBA_MID)) != 0 ||
      io_in8(PATA_REGISTER(bus, PATA_REG_LBA_HI)) != 0) {
        /* found ATAPI device, run away */
        return;
    }

    /* wait until data is ready */
    while (ISUNSET(_pata_read_status(bus),
      (PATA_STATUS_DRQ | PATA_STATUS_ERR)));

    if (ISSET(_pata_read_status(bus), PATA_STATUS_ERR)) {
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

static int
pata_open(devd_t *d, int flags)
{
    struct pata_device *dev = d->priv;
    if (dev->busy != FALSE) {
        return -EBUSY;
    }
    dev->busy = TRUE;
    return 0;
}

static int
pata_close(devd_t *d)
{
    struct pata_device *dev = d->priv;
    dev->busy = FALSE;
    return 0;
}

static int
pata_strategy(devd_t *d, iobuf_t *b)
{
    int s;

    struct pata_device *dev = d->priv;
    s = splbio();
    switch (b->oper) {
    /* XXX: bcount is bounded here */
    case BIO_READ:
        _pata_read_lba28(dev, b->addr, b->blkno, b->bcount);
        break;
    case BIO_WRITE:
        _pata_write_lba28(dev, b->addr, b->blkno, b->bcount);
        break;
    default:
        panic("unknown b->oper");
        /*NOTREACHED*/
    }
    splx(s);
    bio_done(b);
    return 0;
}

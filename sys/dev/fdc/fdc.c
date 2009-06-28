/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
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

/** @file
 * Sterownik kontrolera stacji dyskietek na szynie ISA.
 *
 * Dokumentacja kontrolera:
 *   http://www.isdaman.com/alsos/hardware/fdc/floppy.htm
 *
 * Sterownik mo¿e wydawaæ siê "zbyt elastyczny", ale fajnie by³o spróbowaæ
 * napisaæ jaki¶ kod, który ³atwo by³oby rozszerzyæ do obs³ugi wiêkszej
 * ilo¶ci kontrolerów lub innych portów I/O.
 *
 */


#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/kargs.h>
#include <sys/bio.h>
#include <dev/fdc/fdc.h>
#include <machine/bus/isa.h>
#include <machine/interrupt.h>
#include <machine/io.h>
#include <machine/cmos.h>
#include <machine/atomic.h>

enum {
    FDC_IRQ     = 6,        ///< Numer przerwania
    FDC_IO_PRI  = 0x3f0,    ///< Numer bazowego portu wej-wyj.
    FDC_IO_SEC  = 0x370     ///< Inny numerek bazowego portu wej-wyj :).
};

/// Rejestry kontrolera stacji dyskietek.
enum FDC_IO_REGISTERS {
    IO_REG_STAT_A   = 0,
    IO_REG_STAT_B   = 1,
    IO_REG_DOR      = 2,
    IO_REG_MSR      = 4,
    IO_REG_FIFO     = 5,
    IO_REG_CCR      = 7
};

/// Polecenia
enum FDC_COMMANDS {
    FDC_READT       = 2,    ///< Odczytaj ¶cie¿kê
    FDC_FIXDRV      = 3,
    FDC_STATUS      = 4,
    FDC_WRITE       = 0xC5, ///< Zapisz sektor.
    FDC_READ        = 0xC6, ///< Odczytaj sektor.
    FDC_CALIBRATE   = 7,    ///< Kalibruj.
    FDC_CHECKINTRPT = 8,    ///< Sprawd¼ stan przerwania.
    FDC_WRITED      = 9,
    FDC_READID      = 10,
    FDC_READD       = 12,
    FDC_FORMATT     = 13,
    FDC_SEEK        = 15    ///< Przesuñ g³owicê.
};

enum {
    DRV_A           = 0,
    DRV_B           = 1,
    DRV_C           = 2,
    DRV_D           = 3
};

/// Bity rejestru MSR.
enum MSR_BITS {
    MSR_ACTA        = 1 << 0,   ///< stan silnika stacji A:
    MSR_ACTB        = 1 << 1,   ///< stan silnika stacji B:
    MSR_ACTC        = 1 << 2,   ///< stan silnika stacji C:
    MSR_ACTD        = 1 << 3,   ///< stan silnika stacji D:
    MSR_BUSY        = 1 << 4,
    MSR_NDMA        = 1 << 5,
    MSR_DIO         = 1 << 6,
    MSR_MRQ         = 1 << 7
};

/// Bity rejestru DOR
enum {
    DOR_DR0         = 1 << 0,   ///< wybór stacji
    DOR_DR1         = 1 << 1,   ///< wybór stacji
    DOR_REST        = 1 << 2,   ///< RESET
    DOR_DMA         = 1 << 3,   ///< w³±czone DMA
    DOR_MOTA        = 1 << 4,   ///< w³±czenie silnika stacji A:
    DOR_MOTB        = 1 << 5,   ///< w³±czenie silnika stacji B:
    DOR_MOTC        = 1 << 6,   ///< w³±czenie silnika stacji C:
    DOR_MOTD        = 1 << 7    ///< w³±czenie silnika stacji D:
};

/// Warto¶ci rejestru CCR
enum {
    CCR_500KB       = 0
};

enum {
    MAX_TRANSFER    = 512*36
};

/// Adres sektora na dyskietce.
typedef struct fdsec fdsec_t;
struct fdsec {
    uchar head;
    uchar track;
    uchar sec;
};

/// Opis kontrolera.
typedef struct fdctrl fdctrl_t;
struct fdctrl {
    int             io;     ///< bazowy port wej-wyj
    int             cmd;    ///< ostatnie polecenie
    spinlock_t      busy;   ///<
    bio_queue_t     bioq;   ///< kolejka operacji wej-wyj
    iobuf_t        *cbp;    ///< obecnie obs³ugiwany bufor
    char*          *iobuf;  ///< bufor transferu
    size_t          iosize; ///< rozmiar transferu
    
};

/// Rodzaj stacji dyskietek.
typedef struct fdspec fdspec_t;
struct fdspec {
    const char  *name;      ///< nazwa
    int         heads;      ///< ilo¶æ powierzchni
    int         tracks;     ///< ilo¶æ ¶cie¿ek
    int         sectrack;   ///< ilo¶æ sektorów na ¶cie¿ce
    int         sechead;    ///< ca³kowita liczba sektorów na powierzchni
    int         secdisk;    ///< ca³kowita liczba sektorów na dysku
    int         gap;        ///< odleg³o¶c pomiêdzy sektorami na dysku
};

/// Opis stacji dyskietek.
typedef struct fddrive fddrive_t;
struct fddrive {
    fdspec_t   *spec;       ///< rodzaj
    fdctrl_t   *ctrl;       ///< kontroler
    int unit;               ///< numer jednostki
    char name;              ///< literka do printf'a
    bool busy;              ///<
    devd_t  *devd;          ///< systemowy deskryptor urz±dzenia

};

/// Tablica rodzajów stacji dyskietek.
static fdspec_t specs[] = {
    { NULL, 0, 0, 0, 0 },
    { "360kB 5.25\"",   2, 40,  9,  720/2,  720,  0}, // zdobyæ GAP!
    { "1200kB 5.25\"",  2, 80, 15, 2400/2, 2400,  0}, // zdobyæ GAP!
    { "750kB 3.5\"",    2, 80,  9, 1440/2, 1440, 23},
    { "1440kB 3.5\"",   2, 80, 18, 2880/2, 2880, 23},
    { "2880kB 3.5\"",   2, 80, 36, 5760/2, 5760, 23},
    { NULL, 0, 0, 0, 0 },
    { NULL, 0, 0, 0, 0 }
};


inline uint8_t rdreg(fdctrl_t *drv, int reg);
inline void wrreg(fdctrl_t *drv, int reg, uint8_t val);
inline uint8_t rdfifo(fdctrl_t *drv);
inline void wrfifo(fdctrl_t *drv, uint8_t data );

void fd_create(fddrive_t *fd);
static spinlock_t ilock;
static bool fdinterrupt(void);
int wait_for_intrpt(fdctrl_t *ctrl, uint8_t *a, uint8_t *b);
int blkno_to_fdsec(fddrive_t *drv, blkno_t n, fdsec_t *fds);
void fdc_init(void);
void fdc_reset(fdctrl_t *ctrl);
int fdc_calibrate(fddrive_t *drv);
void fdc_io(fdctrl_t *ctrl);
void fdc_motor_on(fddrive_t *drv);
void fdc_motor_off(fddrive_t *drv);
void fdc_readsec(fddrive_t *drv, iobuf_t *b);

static bus_isa_dma_t *fd_dma;
static fdctrl_t fdctrl;
static fddrive_t fddrive[2];

d_open_t fd_open;
d_close_t fd_close;
d_strategy_t fd_strategy;

static devsw_t fd_devsw = {
    fd_open,
    fd_close,
    noioctl,
    noread,
    nowrite,
    fd_strategy,
    DEV_BDEV,
    "fd"
};

/*========================================================================
 * Operacje I/O
 */


uint8_t
rdreg(fdctrl_t *drv, int reg)
{
    return io_in8(drv->io + reg);
}

void
wrreg(fdctrl_t *drv, int reg, uint8_t val)
{
    io_out8(drv->io + reg, val);
}

uint8_t
rdfifo(fdctrl_t *drv)
{
    while ( ISUNSET(rdreg(drv, IO_REG_MSR), MSR_MRQ) );
    return rdreg(drv, IO_REG_FIFO);
}

void
wrfifo(fdctrl_t *drv, uint8_t data )
{
    while ( ISUNSET(rdreg(drv, IO_REG_MSR), MSR_MRQ) );
    wrreg(drv, IO_REG_FIFO, data);
}

/*========================================================================
 * Pomocnicze procedury
 */


/// T³umaczy numer bloku na jego adres.
int
blkno_to_fdsec(fddrive_t *drv, blkno_t n, fdsec_t *fds)
{
    if (drv->spec->secdisk <= n) {
        return -1;
    }
    fds->track = n / (drv->spec->sectrack * 2);
    blkno_t tn = n % (drv->spec->sectrack * 2);
    fds->head = tn / drv->spec->sectrack;
    fds->sec = tn % drv->spec->sectrack + 1;
    return 0;
}


/*========================================================================
 * Podprogram obs³ugi przerwania.
 *
 *
 */

void fdc_work(fdctrl_t *ctrl);
void fdc_seek(fddrive_t *drv, fdsec_t *sec);
void seek_done(fdctrl_t *ctrl, iobuf_t *bp);
void io_done(fdctrl_t *ctrl, iobuf_t *bp);
/// Obs³uga przerwania.

#include <sys/console.h>
bool
fdinterrupt()
{
    fdctrl_t *ctrl = &fdctrl;
    switch (ctrl->cmd) {
        case FDC_SEEK:
            seek_done(ctrl, ctrl->cbp);
            break;
        case FDC_READ:
        case FDC_WRITE:
            io_done(ctrl, ctrl->cbp);
            fdc_work(ctrl);
            break;
        case -1:
            break;
        case FDC_CALIBRATE:
            break;
    }
    spinlock_unlock(&ilock);
    return 0;
}

void
fdc_work(fdctrl_t *ctrl)
{
    if (ctrl->cbp) return;
    iobuf_t *bp = bioq_dequeue(&ctrl->bioq);
    if (!bp) {
        spinlock_unlock(&ctrl->busy);
        return;
    }
    fdsec_t sec;
    fddrive_t *drv = bp->dev->priv;
    fdc_motor_on(drv);
    if (blkno_to_fdsec(drv, bp->blkno, &sec)) {
        bio_error(bp, EINVAL);
        return;
    }
    ctrl->cmd = FDC_SEEK;
    ctrl->cbp = bp;
    wrfifo(ctrl, ctrl->cmd);
    wrfifo(ctrl, sec.head << 2);
    wrfifo(ctrl, sec.track);
}

void
io_done(fdctrl_t *ctrl, iobuf_t *bp)
{
    if (!bp) return;
    uint8_t st0, st1, st2, track, head, secn, secs;

    st0 = rdfifo(ctrl);
    st1 = rdfifo(ctrl);
    st2 = rdfifo(ctrl);
    track = rdfifo(ctrl);
    head = rdfifo(ctrl);
    secn = rdfifo(ctrl);
    secs = rdfifo(ctrl);

    if (st0 >> 6) {
        bio_error(bp, EIO);
        ctrl->cmd = -1;
        ctrl->cbp = 0;
        return;
    }
    bp->resid -= ctrl->iosize;
    ctrl->iobuf += ctrl->iosize;
    if (bp->resid == 0) {
        bus_isa_dma_finish(fd_dma);
        bio_done(bp);
        ctrl->cmd = -1;
        ctrl->cbp = 0;
    } else {
        fdc_io(ctrl);
    }
}

void
seek_done(fdctrl_t *ctrl, iobuf_t *bp)
{
    if (!bp) return;

    uint8_t st0, cyl;
    wrfifo(ctrl, FDC_CHECKINTRPT);
    st0 = rdfifo(ctrl);
    cyl = rdfifo(ctrl);

    if (st0 >> 6) {
        bio_error(bp, EIO);
        return;
    }

    ctrl->iobuf = bp->addr;
    fdc_io(ctrl);
}



/**
 * Oczekiwanie na przerwanie (aktywne).
 * @param ctrl kontroler stacji dyskietek
 * @param _a do wype³nienia aktualnym numerem sektora
 * @param _b do wype³nienia aktualnym numerem cylindra
 * @return wynik poprzedniej operacji
 *
 * Je¿eli kontroler zosta³ podany to po przerwaniu zostanie do niego
 * wys³ane polecenie sprawdzenia przerwania. To polecenie odpowiada
 * wysy³aj±c dwa bajty, pierwszy to rejestr ST0, drugi to aktualny
 * numer cylindra. Rejestr ST0 zawiera w sobie aktualny numer sektora
 * oraz status wydanego polecenia.
 */
int
wait_for_intrpt(fdctrl_t *ctrl, uint8_t *_a, uint8_t *_b)
{
    uint8_t a, b, res;
    spinlock_lock(&ilock);
    if (ctrl == NULL) return 0;

    wrfifo(ctrl, FDC_CHECKINTRPT);
    a = rdfifo(ctrl); // ST0
    b = rdfifo(ctrl); // cylinder
    if (_a) *_a = a & 0x3f;
    if (_b) *_b = b;
    res = a>>6 & 0x3;
    return (res)? -1 : 0;
}

/*========================================================================
 * Sterownik kontrolera.
 *
 * Polecenia do sterownika id± przez rejestr IO_REG_FIFO. Za ka¿dym
 * poleceniem do rejestru nale¿y zapisaæ jego argumenty.
 */


void
fdc_reset(fdctrl_t *ctrl)
{
    wrreg(ctrl, IO_REG_DOR, 0);
    wrreg(ctrl, IO_REG_DOR, DOR_DMA|DOR_REST);
    wait_for_intrpt(ctrl, 0, 0);
    wrreg(ctrl, IO_REG_CCR, CCR_500KB);

    wrfifo(ctrl, FDC_FIXDRV);
    wrfifo(ctrl, 0xdf);
    wrfifo(ctrl, 0x02);

}

/**
 * Kalibruje stacjê dyskietek.
 * @param drv stacja dyskietek
 *
 * Kalibracja polega na przestawieniu g³owicy na sam pocz±tek.
 */
int
fdc_calibrate(fddrive_t *drv)
{
    fdctrl_t *ctrl = drv->ctrl;
    DEBUGF("callibrating driver %c:", drv->name);
    fdc_motor_on(drv);
    wrfifo(ctrl, FDC_CALIBRATE);
    wrfifo(ctrl, drv->unit);
    if (wait_for_intrpt(ctrl, 0, 0))
        return -ENXIO;
    return 0;
}

void
fdc_motor_on(fddrive_t *drv)
{
    wrreg(drv->ctrl, IO_REG_DOR, DOR_DMA|DOR_REST|1<<(drv->unit+4));
}

void
fdc_motor_off(fddrive_t *drv)
{
    uint8_t r = rdreg(drv->ctrl, IO_REG_DOR);
    UNSET(r, 1 << (drv->unit+4) );
    wrreg(drv->ctrl, IO_REG_DOR, r|DOR_DMA|DOR_REST);
}

void
fdc_io(fdctrl_t *ctrl)
{
    int dma_cmd;
    iobuf_t *bp = ctrl->cbp;
    fddrive_t *drv = bp->dev->priv;
    fdsec_t sec;

    if (blkno_to_fdsec(drv, bp->blkno, &sec)) {
        bio_error(bp, EINVAL);
        ctrl->cmd = -1;
        ctrl->cbp = 0;
        return;
    }

    switch (bp->oper) {
        case BIO_READ:
            ctrl->cmd = FDC_READ;
            dma_cmd = ISA_DMA_READ;
            break;
        case BIO_WRITE:
            ctrl->cmd = FDC_WRITE;
            dma_cmd = ISA_DMA_WRITE;
            break;
        default:
            ctrl->cmd = -1;
            ctrl->cbp = 0;
            bio_error(bp, EIO);
            return;
    }

    ctrl->iosize = MIN(MAX_TRANSFER, bp->resid);
    bus_isa_dma_prepare(fd_dma, dma_cmd, ctrl->iobuf, ctrl->iosize);
    wrfifo(ctrl, ctrl->cmd);
    wrfifo(ctrl, drv->unit | (sec.head << 2));
    wrfifo(ctrl, sec.track);
    wrfifo(ctrl, sec.head);
    wrfifo(ctrl, sec.sec);
    wrfifo(ctrl, 2);
    wrfifo(ctrl, drv->spec->sectrack);
    wrfifo(ctrl, drv->spec->gap);
    wrfifo(ctrl, 0xff);

}

/*========================================================================
 * Inicjalizacja sterownika
 */

void
fdc_init(void)
{
    if (karg_is_set("fdc_disable")) return;

    fdctrl.io = FDC_IO_PRI;
    karg_get_i("fdc_port", &fdctrl.io);

    DEBUGF("fdc port is 0x%x", fdctrl.io);
    DEBUGF("probing cmos");
    // pytamy siê CMOS czy istniej± urz±dzenia.
    uint8_t probe = cmos_rdreg(CMOS_REG_FDC);
    if (probe == 0) {
        DEBUGF("any floppy disk detected");
        return;
    }
    bioq_init(&fdctrl.bioq);
    spinlock_init(&fdctrl.busy);
    fddrive[0].spec = &specs[probe >> 4];
    fddrive[0].unit = 0;
    fddrive[0].ctrl = &fdctrl;
    fddrive[0].name = 'A';
    fddrive[1].spec = &specs[probe & 0xf];
    fddrive[1].unit = 1;
    fddrive[1].ctrl = &fdctrl;
    fddrive[1].name = 'B';

    if (!fddrive[0].spec->name && !fddrive[0].spec->name) {
        DEBUGF("detected floppys are not supported by fdc driver A:%u B:%u",
            probe >> 4, probe & 0xf);
        return;
    }
    // jak jakie¶ stacje s± dostêpne to przydzialny kana³ DMA,
    // instalujemy podprogram przerwania i takie tam
    fd_dma = bus_isa_dma_alloc(ISA_DMA_FDC);
    spinlock_init(&ilock);
    spinlock_lock(&ilock);
    irq_install_handler(FDC_IRQ, fdinterrupt, IPL_BIO);

    fdc_reset(&fdctrl);
    fd_create(&fddrive[0]);
    fd_create(&fddrive[1]);
}

void
fd_create(fddrive_t *fd)
{
    if (!fd->spec->name) return;
    kprintf("fd%u: <%s> drive %c: geometry %u/%u/%u\n", fd->unit,
        fd->spec->name, fd->name, fd->spec->heads, fd->spec->tracks,
        fd->spec->sectrack);
    fd->devd = devd_create(&fd_devsw, fd->unit, fd);
    fd->busy = FALSE;
}

/*========================================================================
 * Obs³uga pliku urz±dzenia blokowego /dev/fdX
 */

int
fd_open(devd_t *d, int flags)
{
    fddrive_t *fd = d->priv;
    if (fd->busy) {
        return -EBUSY;
    }
    fd->busy = TRUE;
    fdc_calibrate(fd);
    return 0;
}

int
fd_close(devd_t *d)
{
    fddrive_t *fd = d->priv;
    fd->busy = FALSE;
    return 0;
}

int
fd_strategy(devd_t *d, iobuf_t *bp)
{
    fddrive_t *fd = d->priv;
    bioq_lock(&fd->ctrl->bioq);
    bioq_enqueue(&fd->ctrl->bioq, bp);
    if (spinlock_trylock(&fd->ctrl->busy)) {
        int s = splbio();
        fdc_work(fd->ctrl);
        splx(s);
    }
    bioq_unlock(&fd->ctrl->bioq);
    return 0;
}

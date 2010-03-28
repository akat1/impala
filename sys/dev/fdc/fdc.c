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
 * Sterownik może wydawać się "zbyt elastyczny", ale fajnie było spróbować
 * napisać jakiś kod, który łatwo byłoby rozszerzyć do obsługi większej
 * ilości kontrolerów lub innych portów I/O.
 *
 */


#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/kargs.h>
#include <sys/bio.h>
#include <dev/fdc/fdc.h>
#include <fs/devfs/devfs.h>
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
    FDC_READT       = 2,    ///< Odczytaj ścieżkę
    FDC_FIXDRV      = 3,
    FDC_STATUS      = 4,
    FDC_WRITE       = 0xC5, ///< Zapisz sektor.
    FDC_READ        = 0xC6, ///< Odczytaj sektor.
    FDC_CALIBRATE   = 7,    ///< Kalibruj.
    FDC_CHECKINTRPT = 8,    ///< Sprawdź stan przerwania.
    FDC_WRITED      = 9,
    FDC_READID      = 10,
    FDC_READD       = 12,
    FDC_FORMATT     = 13,
    FDC_SEEK        = 15    ///< Przesuń głowicę.
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
    DOR_DMA         = 1 << 3,   ///< włączone DMA
    DOR_MOTA        = 1 << 4,   ///< włączenie silnika stacji A:
    DOR_MOTB        = 1 << 5,   ///< włączenie silnika stacji B:
    DOR_MOTC        = 1 << 6,   ///< włączenie silnika stacji C:
    DOR_MOTD        = 1 << 7    ///< włączenie silnika stacji D:
};


#define ST0_IC(st0) (st0>>6)
/// bity IC z ST0
enum {
    ST0_OK      = 0x0,
    ST0_ABNRM   = 0x1,
    ST0_INVAL   = 0x2,
    ST0_NREADY  = 0x3
};

/// bity rejestru ST1
enum {
    ST1_NID     = 1 << 0,
    ST1_NW      = 1 << 1,
    ST1_NDAT    = 1 << 2,
    ST1_TO      = 1 << 4,
    ST1_DE      = 1 << 5,
    ST1_EN      = 1 << 7
};

/// Wartości rejestru CCR
enum {
    CCR_500KB       = 0
};

enum {
    MAX_TRANSFER    = 512,
    MAX_RETRIES     = 8
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
    iobuf_t        *cbp;    ///< obecnie obsługiwany bufor
    char           *iobuf;  ///< bufor transferu
    size_t          iosize; ///< rozmiar transferu
    int             retry;  ///< 
    blkno_t         blkno;  ///< aktualny blok
    bus_isa_dma_t  *dma;    ///< deskryptor ISA DMA
    fdsec_t         pos;    ///< pozycja głowicy    
};

/// Rodzaj stacji dyskietek.
typedef struct fdspec fdspec_t;
struct fdspec {
    const char  *name;      ///< nazwa
    int         heads;      ///< ilość powierzchni
    int         tracks;     ///< ilość ścieżek
    int         sectrack;   ///< ilość sektorów na ścieżce
    int         sechead;    ///< całkowita liczba sektorów na powierzchni
    int         secdisk;    ///< całkowita liczba sektorów na dysku
    int         gap;        ///< odległośc pomiędzy sektorami na dysku
};

/// Opis stacji dyskietek.
typedef struct fddrive fddrive_t;
struct fddrive {
    fdspec_t   *spec;       ///< rodzaj
    fdctrl_t   *ctrl;       ///< kontroler
    int unit;               ///< numer jednostki
    char name;              ///< literka do printf'a
    bool busy;              ///<
    devd_t  *devd;          ///< systemowy deskryptor urządzenia

};

/// Tablica rodzajów stacji dyskietek.
static fdspec_t specs[] = {
    { NULL, 0, 0, 0, 0 },
    { "360kB 5.25\"",   2, 40,  9,  720/2,  720,  0}, // zdobyć GAP!
    { "1200kB 5.25\"",  2, 80, 15, 2400/2, 2400,  0}, // zdobyć GAP!
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
static int wait_for_intrpt(fdctrl_t *ctrl, uint8_t *a, uint8_t *b);
static int blkno_to_fdsec(fddrive_t *drv, blkno_t n, fdsec_t *fds);
void fdc_init(void);
static void fdc_reset(fdctrl_t *ctrl);
static int fdc_calibrate(fddrive_t *drv);
static void fdc_io(fdctrl_t *ctrl);
static void fdc_motor_on(fddrive_t *drv);
void fdc_motor_off(fddrive_t *drv);
static void fdc_work(fdctrl_t *ctrl);
static void fdc_seek(fdctrl_t *ctrl);
static void seek_done(fdctrl_t *ctrl, iobuf_t *bp);
static void io_done(fdctrl_t *ctrl, iobuf_t *bp);

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


/// Tłumaczy numer bloku na jego adres.
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
 * Podprogram obsługi przerwania.
 *
 *
 */

/// Obsługa przerwania.
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
io_done(fdctrl_t *ctrl, iobuf_t *bp)
{
    uint8_t st0, st1, st2, track, head, secn, secs;

    st0 = rdfifo(ctrl);
    st1 = rdfifo(ctrl);
    st2 = rdfifo(ctrl);
    track = rdfifo(ctrl);
    head = rdfifo(ctrl);
    secn = rdfifo(ctrl);
    secs = rdfifo(ctrl);

    if (ST0_IC(st0)) {
        const char *msg;
        int ee =  ST0_IC(st0);
        msg = (ee==ST0_ABNRM)? "abnormal termination"
                : (ee==ST0_INVAL)? "invalid command"
                : (ee==ST0_NREADY)? "not ready"
                : "#!?#";
        if (ctrl->retry == MAX_RETRIES) {
            IDEBUGF("I/O error st0=%b(%x) st1=%b(%x) st2=%b(%x)",
                st0,st0,st1,st1,st2,st2);
            IDEBUGF(" request: blkno=%p size=%p resid=%p", bp->blkno,
                bp->size, bp->resid);
            IDEBUGF("transfer: blkno=%p size=%p; after %u retries",
                ctrl->blkno, ctrl->iosize, ctrl->retry);
            bio_error(bp, EIO);
            ctrl->cmd = -1;
            ctrl->cbp = 0; 
        } else {
            ctrl->retry++;
            fdc_seek(ctrl);
        }
        return;
    }
    bus_isa_dma_finish(ctrl->dma);
    KASSERT(bp->resid > 0);
    KASSERT((size_t)ctrl->iobuf < (size_t)bp->addr + bp->size);
    KASSERT(ctrl->blkno < bp->blkno + bp->bcount);
    bp->resid -= ctrl->iosize;
    ctrl->iobuf += ctrl->iosize;
    ctrl->blkno += ctrl->iosize/512;
    ctrl->retry = 0;
    if (bp->resid == 0) {
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
    uint8_t st0, cyl;
    if (!bp) return;
    wrfifo(ctrl, FDC_CHECKINTRPT);
    st0 = rdfifo(ctrl);
    cyl = rdfifo(ctrl);

    if (st0 >> 6) {
        bio_error(bp, EIO);
        return;
    }

    fdc_io(ctrl);
}



/**
 * Oczekiwanie na przerwanie (aktywne).
 * @param ctrl kontroler stacji dyskietek
 * @param _a do wypełnienia aktualnym numerem sektora
 * @param _b do wypełnienia aktualnym numerem cylindra
 * @return wynik poprzedniej operacji
 *
 * Jeżeli kontroler został podany to po przerwaniu zostanie do niego
 * wysłane polecenie sprawdzenia przerwania. To polecenie odpowiada
 * wysyłając dwa bajty, pierwszy to rejestr ST0, drugi to aktualny
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
 * Polecenia do sterownika idą przez rejestr IO_REG_FIFO. Za każdym
 * poleceniem do rejestru należy zapisać jego argumenty, inaczej nastąpi
 * zwiecha FDC.
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

void
fdc_work(fdctrl_t *ctrl)
{
    if (ctrl->cbp) return;
    iobuf_t *bp = bioq_dequeue(&ctrl->bioq);
    if (!bp) {
        spinlock_unlock(&ctrl->busy);
        return;
    }
    ctrl->retry = 0;
    ctrl->cbp = bp;
    ctrl->blkno = bp->blkno;
    ctrl->iobuf = bp->addr;
    fdc_seek(ctrl);
}

void
fdc_seek(fdctrl_t *ctrl)
{
    iobuf_t *bp = ctrl->cbp;
    fdsec_t sec;
    fddrive_t *drv = bp->dev->priv;

    fdc_motor_on(drv);
    if (blkno_to_fdsec(drv, ctrl->blkno, &sec)) {
        bio_error(bp, EINVAL);
        return;
    }
    ctrl->cmd = FDC_SEEK;
    wrfifo(ctrl, ctrl->cmd);
    wrfifo(ctrl, sec.head << 2);
    wrfifo(ctrl, sec.track);
}


void
fdc_io(fdctrl_t *ctrl)
{
    int dma_cmd;
    iobuf_t *bp = ctrl->cbp;
    fddrive_t *drv = bp->dev->priv;
    fdsec_t sec;
    if (blkno_to_fdsec(drv, ctrl->blkno, &sec)) {
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

    blkno_t n;
    if (ctrl->retry == 0) {
        n = drv->spec->sectrack * drv->spec->heads;
        n = n - ctrl->blkno%n;
    } else {
        n = 1;
    }
    ctrl->iosize = MIN(n*512, bp->resid);
    bus_isa_dma_prepare(ctrl->dma, dma_cmd, ctrl->iobuf, ctrl->iosize);
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

/**
 * Kalibruje stację dyskietek.
 * @param drv stacja dyskietek
 *
 * Kalibracja polega na przestawieniu głowicy na sam początek.
 */
int
fdc_calibrate(fddrive_t *drv)
{
    fdctrl_t *ctrl = drv->ctrl;
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
    // pytamy się CMOS czy istnieją urządzenia.
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
///@bug proszę o weryfikację tego if-a
    if (!fddrive[0].spec->name && !fddrive[0].spec->name) {
        DEBUGF("detected floppys are not supported by fdc driver A:%u B:%u",
            probe >> 4, probe & 0xf);
        return;
    }
    // jak jakieś stacje są dostępne to przydzialny kanał DMA,
    // instalujemy podprogram przerwania i takie tam
    fdctrl.dma = bus_isa_dma_alloc(ISA_DMA_FDC);
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
    fd->devd = devd_create(&fd_devsw, "fd", fd->unit, fd);
    devfs_register(fd->devd, 0, 0, 0666);
    fd->busy = FALSE;
}

/*========================================================================
 * Obsługa pliku urządzenia blokowego /dev/fdX
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

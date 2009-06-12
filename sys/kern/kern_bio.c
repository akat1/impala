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

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/stat.h>
#include <sys/kargs.h>
#include <sys/thread.h>
#include <sys/kthread.h>
#include <sys/bio.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/kmem.h>
#include <sys/device.h>
#include <sys/utils.h>
#include <sys/vfs.h>
#include <machine/interrupt.h>
// #define splbio() 1
// #define splx(x) x = 1
/** @file
 * Ten plik zawiera implementacjê procedur odpowiedzialnych za wej¶cie-wyj¶cie
 * na urz±dzeniach blokowych. Ten mechanizm (jak wiele innych) jest wzorowany
 * na UNIXie (BSD: kern/vfs_bio.c, Solaris: os/bio.c).
 *
 * System przechowuje w pamiêci dane z urz±dzenia blokowego w celu
 * minimalizacji wolnych operacji wej¶cia-wyj¶cia. Pamiêc podrêczna jest
 * ograniczona do NBUF buforów, które s± trzymane w tablicy haszuj±cej
 * bufhash. Sta³e BUFHASH_* s± parametrami funkcji haszuj±cej.
 *
 * Wielko¶æ buforów mo¿e zostaæ zmieniona przy starcie systemu przez
 * argument "iobufs".
 */

enum {
    BUFHASH_P1      = 461,      ///< parametr 'a' funkcji haszuj±cej
    BUFHASH_P2      = 812,      ///< parametr 'b' funkcji haszuj±cej
    BUFHASH_P_M     = 701,      ///< liczba pierwsza 'm'
    BUFHASH_P_P     = 1117,     ///< liczba pierwsza 'p'
    NBUF            = 4096      ///< ilo¶æ buforów w pamiêci podrêcznej
};

// f(dev,blkno) = ((a*k + b) mod p) mod m
// gdzie k = (dev+1)*(blkno+1)
#define __(k) (((BUFHASH_P1*(k) + BUFHASH_P2) % BUFHASH_P_P) % BUFHASH_P_M)
#define ___(a,b) ((a+1)*(b+1))
#define BUFHASH(dev,blkno) &bufhash.queues[__(___((uintptr_t)dev, blkno))]


struct bufhash {
    mutex_t     mtx;
    list_t      queues[BUFHASH_P_M];
    list_t      free;
};

static iobuf_t *bufhash_find(devd_t *dev, blkno_t b);
static iobuf_t *bufhash_getfree(void);
static void bufhash_insert(iobuf_t *bp);
static void bufhash_remove(iobuf_t *bp);
static void bufhash_putfree_head(iobuf_t *bp);
void bufhash_putfree_tail(iobuf_t *bp);

static void buf_alloc(iobuf_t *bp, devd_t *d, blkno_t n, size_t bsize);
static void buf_ctor(void *x);

static kmem_cache_t *buf_cache;
static struct bufhash bufhash;

static int nbufs = NBUF;

void
bio_init()
{
    buf_cache = kmem_cache_create("biobuf", sizeof(iobuf_t), buf_ctor, NULL);

    mutex_init(&bufhash.mtx, MUTEX_CONDVAR);
    for (int i = 0; i < BUFHASH_P_M; i++) {
        LIST_CREATE(&bufhash.queues[i], iobuf_t, L_hash, FALSE);
    }
    karg_get_i("iobufs", &nbufs);
    DEBUGF("input/output buffers count=%u", nbufs);
    LIST_CREATE(&bufhash.free, iobuf_t, L_free, FALSE);
    for (int i = 0; i < nbufs; i++) {
        iobuf_t *bp = kmem_cache_alloc(buf_cache, KM_SLEEP);
        mem_zero(bp, sizeof(*bp));
        list_insert_tail(&bufhash.free, bp);
    }
}


/*============================================================================
 * Procedury odpowiedzialne za zarz±dzanie tablic± haszuj±c±.
 */

iobuf_t *
bufhash_find(devd_t *dev, blkno_t b)
{
    mutex_lock(&bufhash.mtx);
    list_t *l = BUFHASH(dev, b);
//     DEBUGF("BUFHASH FIND dev=/dev/%s blkno=%u hash=%u", dev->name,
//         b, __(___( (uintptr_t)dev, b)));
    iobuf_t *bp = NULL;
    while ( (bp = list_next(l, bp)) ) {
        if (bp->dev != dev || bp->blkno != b) continue;
        mutex_unlock(&bufhash.mtx);
        return bp;
    }
    mutex_unlock(&bufhash.mtx);
    return NULL;
}

iobuf_t *
bufhash_getfree()
{
    mutex_lock(&bufhash.mtx);
    iobuf_t *bp = NULL;
    while ( list_length(&bufhash.free) == 0 ) {
        mutex_wait(&bufhash.mtx);
    }
    int s = splbio();
    bp = list_extract_first(&bufhash.free);
    UNSET(bp->flags, BIO_VALID);
    splx(s);
    mutex_unlock(&bufhash.mtx);
    return bp;
}

void
bufhash_insert(iobuf_t *bp)
{
    list_t *l = BUFHASH(bp->dev, bp->blkno);
    mutex_lock(&bufhash.mtx);
    list_insert_tail(l, bp);
    SET(bp->flags, BIO_CACHE);
    mutex_unlock(&bufhash.mtx);
}

void
bufhash_remove(iobuf_t *bp)
{
    if ( ISUNSET(bp->flags, BIO_CACHE) ) return;
    list_t *l = BUFHASH(bp->dev, bp->blkno);
    mutex_lock(&bufhash.mtx);
    list_remove(l, bp);
    UNSET(bp->flags,BIO_CACHE);
    mutex_unlock(&bufhash.mtx);
}

void
bufhash_putfree_head(iobuf_t *bp)
{
    mutex_lock(&bufhash.mtx);
    int s = splbio();
    list_insert_head(&bufhash.free, bp);
    splx(s);
    mutex_unlock(&bufhash.mtx);
}

void
bufhash_putfree_tail(iobuf_t *bp)
{
    mutex_lock(&bufhash.mtx);
    int s = splbio();
    list_insert_tail(&bufhash.free, bp);
    splx(s);
    mutex_unlock(&bufhash.mtx);
}

/*============================================================================
 * Procedury odpowiedzialne za zarz±dzanie buforami.
 */


void
buf_ctor(void *x)
{
    iobuf_t *bp = x;
    bp->flags = 0;
    bp->size = 0;
    bp->addr = NULL;
    sleepq_init(&bp->sleepq);
}

void
buf_alloc(iobuf_t *bp, devd_t *d, blkno_t n, size_t bsize)
{
    bp->dev = d;
    bp->blkno = n;
    bp->bcount = 1;
    // rozmiar bufora siê zgadza, no to po robocie.
    if (bp->size == bsize) return;
    UNSET(bp->flags, BIO_DONE);
    void *a = kmem_alloc(bsize, KM_SLEEP);
    // zmieniamy bufor, zachowuj±c czê¶c danych.
    if (bp->addr && bp->size < bsize) {
        mem_cpy(a, bp->addr, bp->size);
        kmem_free(bp->addr);
        bp->addr = a;
    } else
    if (bp->addr) {
        UNSET(bp->flags, BIO_VALID);
        kmem_free(bp->addr);
        bp->addr = a;
    }
    bp->addr = a;
    bp->size = bsize;
//     ssleep(1);
}

#if 0
void
buf_destroy(iobuf_t *bp)
{
    if (bp->addr) kmem_free(bp->addr);
    bp->dev = 0;
    bp->addr = NULL;
}
#endif
/*============================================================================
 * Interfejs zewnêtrzny BIO
 */


iobuf_t *
bio_getblk(devd_t *d, blkno_t n)
{
    iobuf_t *bp = NULL;
    do {
        bp = bufhash_find(d, n);
        if (bp) {
            if ( ISSET(bp->flags,BIO_BUSY) ) {
//                 DEBUGF("getblk blk %u on %s found BUSY buffer in cache",
//                     n, d->name);
                SET(bp->flags,BIO_WANTED);
                sleepq_wait(&bp->sleepq);
                bp = NULL;
            } else {
//                 DEBUGF("getblk blk %u on %s found buffer in cache",
//                     n, d->name);
                return bp;
            }
        } else {
//             DEBUGF("getblk blk %u on %s not found buffer in cache",
//                 n, d->name);
            bp = bufhash_getfree();
            bufhash_remove(bp);
        }
    } while (bp == NULL);

    if ( ISSET(bp->flags,BIO_DELWRI) ) {
        bio_write(bp);
    }
    buf_alloc(bp, d, n, 512);
    bufhash_insert(bp);
    return bp;
}

iobuf_t *
bio_read(devd_t *d, blkno_t n)
{
    iobuf_t *bp = bio_getblk(d, n);
    if ( ISUNSET(bp->flags,BIO_VALID) ) {
//         DEBUGF("read blk %u on %s buffer data is invalid, starting I/O",
//             n, d->name);
        bp->oper = BIO_READ;
        UNSET(bp->flags, BIO_DONE);
        devd_strategy(d, bp);
        bio_wait(bp);
    }
    return bp;
}

void
bio_write(iobuf_t *bp)
{
    bp->oper = BIO_WRITE;
    devd_strategy(bp->dev, bp);
    if ( ISSET(bp->flags,BIO_DELWRI) ) {
        bufhash_putfree_head(bp);
    } else {
        bio_wait(bp);
        bio_release(bp);
    }

}

void
bio_release(iobuf_t *bp)
{
    int s = splbio();
    UNSET(bp->flags,BIO_BUSY|BIO_DONE);
    bufhash_putfree_tail(bp);
    sleepq_wakeup(&bp->sleepq);
    splx(s);
}

void
bio_done(iobuf_t *bp)
{
    int s = splbio();
    SET(bp->flags,BIO_DONE);
    sleepq_wakeup(&bp->sleepq);
    splx(s);
}

void
bio_error(iobuf_t *bp, int error)
{
    int s = splbio();
    bp->errno = error;
    SET(bp->flags,BIO_DONE|BIO_ERROR);
    sleepq_wakeup(&bp->sleepq);
    splx(s);
}


void
bio_wait(iobuf_t *bp)
{
    int s = splbio();
    if ( ISUNSET(bp->flags,BIO_DONE) ) {
        sleepq_wait(&bp->sleepq);
    }
    splx(s);
}

/*============================================================================
 * Interfejs zewnêtrzny PHYSIO
 */

ssize_t
physio(devd_t *dev, uio_t *uio, int bioflags)
{
    return -ENOTSUP;
}

/*============================================================================
 * Interfejs kolejek BIO
 */


void
bioq_init(bio_queue_t *q)
{
    LIST_CREATE(&q->bq_queue, iobuf_t, L_bioq, FALSE);
    mutex_init(&q->bq_mtx, MUTEX_NORMAL);
}

/**
 * Wstawia bufor w kolejkê.
 * @bp bufor.
 *
 * Procedura wy³±cza przerwania zwi±zane z obs³ug± urz±dzeñ blokowych
 * na czas wstawiania w kolejkê w celu ochrony struktury przed uszkodzeniem.
 */
void
bioq_enqueue(bio_queue_t *q, iobuf_t *bp)
{
    int s = splbio();
    list_insert_tail(&q->bq_queue, bp);
    splx(s);
}

/**
 * Pobiera bufor z kolejki.
 * @return bufor.
 *
 * Procedura nie u¿ywa synchronizacji, dziêki czemu mo¿na j± u¿ywaæ wewn±trz
 * przerwania.
 */
iobuf_t *
bioq_dequeue(bio_queue_t *q)
{
    return list_extract_first(&q->bq_queue);
}

void
bioq_lock(bio_queue_t *q)
{
    mutex_lock(&q->bq_mtx);
}

void
bioq_unlock(bio_queue_t *q)
{
    mutex_unlock(&q->bq_mtx);
}


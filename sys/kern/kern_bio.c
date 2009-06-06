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

/*
 * Ten plik zawiera implementacjê procedur odpowiedzialnych za wej¶cie-wyj¶cie
 * na urz±dzeniach blokowych. Ten mechanizm (jak wiele innych) jest wzorowany
 * na UNIXie (BSD: kern/vfs_bio.c, Solaris: os/bio.c).
 *
 * System przechowuje w pamiêci dane z urz±dzenia blokowego w celu
 * minimalizacji wolnych operacji wej¶cia-wyj¶cia. Pamiêc podrêczna jest
 * ograniczona do NBUF buforów, które s± trzymane w tablicy haszuj±cej
 * bufhash. Sta³e BUFHASH_* s± parametrami funkcji haszuj±cej.
 *
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


void
bio_init()
{
    buf_cache = kmem_cache_create("biobuf", sizeof(iobuf_t), buf_ctor, NULL);
    TRACE_IN("allocating %u buffer header for cache", NBUF);

    mutex_init(&bufhash.mtx, MUTEX_CONDVAR);
    for (int i = 0; i < BUFHASH_P_M; i++) {
        LIST_CREATE(&bufhash.queues[i], iobuf_t, L_hash, FALSE);
    }
    LIST_CREATE(&bufhash.free, iobuf_t, L_free, FALSE);
    for (int i = 0; i < NBUF; i++) {
        iobuf_t *bp = kmem_cache_alloc(buf_cache, KM_SLEEP);
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
    TRACE_IN("dev=%p blkno=%u hash=%u", dev, b, __(___( (uintptr_t)dev, b)));
    iobuf_t *bp = NULL;
    while ( (bp = list_next(l, bp)) ) {
        if (bp->dev != dev && bp->blkno != b) continue;
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
    if ( UNSET(bp->flags, BIO_CACHE) ) return;
    list_t *l = BUFHASH(bp->dev, bp->blkno);
    mutex_lock(&bufhash.mtx);
    list_insert_tail(l, bp);
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
    void *a = kmem_alloc(bsize, KM_SLEEP);
    // zmieniamy bufor, zachowuj±c czê¶c danych.
    if (bp->addr && bp->size < bsize) {
        mem_cpy(a, bp->addr, bp->size);
        kmem_free(bp->addr);
        bp->addr = a;
    } else
    if (bp->addr) {
        mem_cpy(a, bp->addr, bsize);
        kmem_free(bp->addr);
        bp->addr = a;
    }
    bp->addr = a;
    bp->size = bsize;
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
        TRACE_IN("trying to get buffer from cache");
        bp = bufhash_find(d, n);
        if (bp) {
            TRACE_IN("found buffer in cache");
            if ( ISSET(bp->flags,BIO_BUSY) ) {
                TRACE_IN("buffer in use, waiting");
                sleepq_wait(&bp->sleepq);
                bp = NULL;
                continue;
            }
        } else {
            TRACE_IN("need new buffer");
            bp = bufhash_getfree();
            bufhash_remove(bp);
        }
    } while (bp == NULL);

    if (bp->flags & BIO_DELWRI) {
        TRACE_IN("delayed write detected");
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
        TRACE_IN("buffer is invalid, starting I/O");
        bp->oper = BIO_READ;
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
    bp->flags &= ~BIO_BUSY;
    sleepq_wakeup(&bp->sleepq);
}

void
bio_done(iobuf_t *bp)
{
    TRACE_IN("bp=%p");
    bp->flags |= BIO_DONE;
    sleepq_wakeup(&bp->sleepq);
}

void
bio_wait(iobuf_t *bp)
{
    if (bp->flags & BIO_DONE) return;
    sleepq_wait(&bp->sleepq);
}

/*============================================================================
 * Interfejs zewnêtrzny PHYSIO
 */

ssize_t
physio(devd_t *dev, uio_t *uio, int bioflags)
{
    return -ENOTSUP;
}


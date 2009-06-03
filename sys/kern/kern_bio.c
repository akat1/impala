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

static void biodaemon_main(void*);
static void biodaemon_sync(void);

static kthread_t biodaemon_thread;

static list_t  bufs_lru;
static list_t  bufs_age0;
static list_t  bufs_age1;
static list_t *bufs_age;
static list_t *bufs_out;
static mutex_t list_lock;

static kmem_cache_t *physbuf_cache;
static void physbuf_ctor(iobuf_t *b);
static void physbuf_dtor(iobuf_t *b);
static int physbuf_init(iobuf_t *b, uio_t *uio);
static void buf_create(iobuf_t *b);
static void buf_destroy(iobuf_t *b);
static void buf_assign(iobuf_t *b, vnode_t *v);
static iobuf_t *buf_alloc(vnode_t *vn, blkno_t b);

void
bio_init()
{
    TRACE_IN0();
    list_create(&bufs_lru, 0, FALSE);
    list_create(&bufs_age0, 0, FALSE);
    list_create(&bufs_age1, 0, FALSE);
    bufs_age = &bufs_age0;
    bufs_out = &bufs_age1;
    mutex_init(&list_lock, MUTEX_NORMAL);

    physbuf_cache = kmem_cache_create("physio",
        sizeof(iobuf_t), (kmem_ctor_t*) physbuf_ctor,
        (kmem_dtor_t*)physbuf_dtor);

    kthread_create(&biodaemon_thread, biodaemon_main, NULL);

}

/*============================================================================
 * Operacje na buforach.
 */

void
buf_create(iobuf_t *b)
{
    semaph_init(&b->sem);
}

void
buf_destroy(iobuf_t *b)
{
    semaph_destroy(&b->sem);
}

void
buf_assign(iobuf_t *b, vnode_t *vn)
{
    KASSERT(vn->v_type = VNODE_TYPE_DEV);
    b->dev = vn->v_dev;
    b->flags |= BIO_BUSY;
}

iobuf_t *
buf_alloc(vnode_t *vn, blkno_t blk)
{
    iobuf_t *bp = kmem_alloc(sizeof(*bp), KM_SLEEP);
    buf_create(bp);
    bp->addr = kmem_alloc(1*512, KM_SLEEP);
    bp->bcount = 1;
    bp->flags = 0;
    bp->blkno = blk;
    return bp;
}

iobuf_t *
bio_getblk(vnode_t *vn, blkno_t blk)
{
    return NULL;
}

iobuf_t *
bio_read(vnode_t *vn, blkno_t blk)
{
    iobuf_t *bp;

    bp = bio_getblk(vn, blk);
    // znaleziono blok w cache
    if (bp) {
        DEBUGF("found buffer in cache");
        buf_assign(bp, vn);
    } else {
        bp = buf_alloc(vn, blk);
        bp->oper = BIO_READ;
        buf_assign(bp, vn);
//        DEBUGF("starting I/O operation for %p %s (%p+%u)", bp->dev->name,
//            bp->dev->name, bp->addr, bp->bcount*512);
        devd_strategy(bp->dev, bp);
        bio_wait(bp);
    }
    return bp;
}

void
bio_wait(iobuf_t *b)
{
    semaph_wait(&b->sem);
}

void
bio_wakeup(iobuf_t *b)
{
    semaph_post(&b->sem);
}

void
bio_done(iobuf_t *b)
{
    b->flags |= BIO_DONE;
//    DEBUGF("I/O operation done (%p+%u)",
//        b->addr, b->bcount*512 );
    bio_wakeup(b);
}


/*============================================================================
 * Bezpo¶rednie operacje I/O (omijaj±ce systemow± pamiêæ podrêczn±)
 */

void
physbuf_ctor(iobuf_t *b)
{
    buf_create(b);
}

void
physbuf_dtor(iobuf_t *b)
{
    buf_destroy(b);
}

int
physbuf_init(iobuf_t *bp, uio_t *uio)
{
    if (uio->space == UIO_USERSPACE) return -1;
    if (uio->iovcnt != 1) return -1;
    if (uio->size != 512) return -1;
    bp->flags = 0;
    bp->bcount = 1;
    bp->addr = uio->iovs[0].iov_base;
    bp->oper = (uio->oper == UIO_READ)? BIO_READ: BIO_WRITE;
    return 0;
}

size_t
physio(devd_t *dev, uio_t *uio, int bioflags)
{
    iobuf_t *bp = kmem_cache_alloc(physbuf_cache, KM_SLEEP);
    int e = physbuf_init(bp, uio);
    if (e != -1) {
        bp->flags |= bioflags;
        e = -1;
    }
    kmem_cache_free(physbuf_cache, bp);
    return e;
}


/*============================================================================
 * Proces systemowy biodaemon
 */

void
biohash_init(biohash_t *h , int n)
{
    h->bh_n = n;
    h->bh_queues = kmem_alloc(n*sizeof(list_t), KM_SLEEP);
    for (int i = 0; i < n; i++) {
        LIST_CREATE(&h->bh_queues[i], iobuf_t, L_hash, FALSE);
    }
    LIST_CREATE(&h->bh_freebufs, iobuf_t, L_free, FALSE);
    mutex_init(&h->bh_mtx, MUTEX_NORMAL);
}

iobuf_t *
biohash_find(biohash_t *h, blkno_t n)
{
    mutex_lock(&h->bh_mtx);
    list_t *q = &h->bh_queues[n%h->bh_n];
    iobuf_t *bp = NULL;
    while ( (bp = list_next(q, bp)) ) {
        if (bp->blkno == n) break;
    }
    mutex_unlock(&h->bh_mtx);
    return bp;
}

void
biohash_insert(biohash_t *h, iobuf_t *bp)
{
    mutex_lock(&h->bh_mtx);
    list_t *l = &h->bh_queues[bp->blkno%h->bh_n];
    list_insert_head(l, bp);
    mutex_unlock(&h->bh_mtx);
}

/*============================================================================
 * Proces systemowy biodaemon
 */

void
biodaemon_main(void *arg)
{
    DEBUGF("biodaemon started");
    for (;;) {
        ssleep(30);
        biodaemon_sync();
    }
}

void
biodaemon_sync()
{
}



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
#include <sys/device.h>
#include <sys/utils.h>
#include <sys/kmem.h>
#include <sys/errno.h>
#include <sys/thread.h>
#include <sys/string.h>
#include <sys/bio.h>
#include <dev/md/md.h>
#include <dev/md/md_priv.h>
#include <fs/devfs/devfs.h>

static d_open_t mdopen;
static d_close_t mdclose;
static d_ioctl_t mdioctl;
static d_read_t mdread;
static d_write_t mdwrite;
static d_strategy_t mdstrategy;

static devsw_t md_devsw = {
    mdopen,
    mdclose,
    mdioctl,
    mdread,
    mdwrite,
    mdstrategy,
    "md"
};

int
mdopen(devd_t *d, int flags)
{
    memdisk_t *md = d->priv;
//    TRACE_IN("d=%p md=%p owner=%p", d, md, md->owner);
    if (md->owner) return -EBUSY;
    md->owner = curthread;
    return 0;
}

int
mdclose(devd_t *d)
{
    memdisk_t *md = d->priv;
    md->owner = NULL;
    return 0;
}

int
mdread(devd_t *d, uio_t *uio)
{
//    TRACE_IN("dev=%p uio=%p", d,uio);
    return physio(d, uio, 0);
}

int
mdwrite(devd_t *d, uio_t *uio)
{
//    TRACE_IN("dev=%p uio=%p", d, uio);
    return physio(d, uio, 0);
}

int
mdioctl(devd_t *dev, int cmd, uintptr_t arg)
{
    return -ENOTTY;
}

int
mdstrategy(devd_t *dev, iobuf_t *b)
{
//    TRACE_IN("dev=%p buf=%p", dev, b);
    memdisk_t *md = dev->priv;

    off_t off = b->blkno*512;
    size_t len = MIN(b->bcount*512, md->size - off);
    if (len < 0) {
        b->flags |= IOB_ERROR;
    }
    if (b->oper == BIO_READ) {
/*
        DEBUGF("memory disk read (%p -> %p) %u bytes",
            md->data+off, b->addr, len);
*/
        mem_cpy(b->addr, md->data + off, len);
    } else {
/*
        DEBUGF("memory disk write (%p -> %p) %u bytes",
            b->addr, md->data+off, len);
*/
        mem_cpy(md->data + off, b->addr, len);
    }
    bio_done(b);
    return 0;
}

/*=========================================================================
 * Zewnêtrzne procedury.
 */

static bool md_less(const memdisk_t *, const memdisk_t *);
static bool md_is_this(const memdisk_t *, uintptr_t unit);
static list_t memdisks;

bool
md_less(const memdisk_t *mdA, const memdisk_t *mdB)
{
    return mdA->unit < mdB->unit;
}

bool
md_is_this(const memdisk_t *md, uintptr_t unit)
{
    return (md->unit == unit);
}

/**
 * Tworzy "pamiêciowy" dysk twardy.
 * @param unit jednostka dysku (/dev/mdXX) 
 * @param data obraz, gdy NULL to sam alokuje dane
 * @param size wielko¶c w bajtach.
 */
int
md_create(int unit, void *data, size_t size)
{
    const char *_str_type;
    memdisk_t *md = NULL;
    bool findunit = (unit==-1);
    while ( (list_next(&memdisks, md)) ) {
        if (findunit) {
            if (unit == md->unit) {
                unit++;
            }
        } else {
            if (md->unit > unit) break;
            if (md->unit == unit) return -1;
        }
    }
    md = kmem_alloc(sizeof(memdisk_t), KM_SLEEP);
    md->unit = unit;
    md->size = size;
    if (data == NULL) {
        md->data_type = MD_DATA_TYPE_ALLOCATED;
        md->data = kmem_alloc(size, KM_SLEEP);
        _str_type = "alloc";
    } else {
        md->data_type = MD_DATA_TYPE_FOREIGN;
        md->data = data;
        _str_type = "foreign";
    }
    list_insert_in_order(&memdisks, md, (list_less_f*) md_less);
    md->devd = devd_create(&md_devsw, md->unit, md);
    devfs_register(md->devd->name, md->devd, 0, 0, 0777);
    md->owner = NULL;       
    return 0;
}

void
md_destroy(int unit)
{
    memdisk_t *md = list_find(&memdisks, md_is_this, unit);
    KASSERT(md != NULL);
    KASSERT(md->owner == NULL);
    list_remove(&memdisks, md);
    if (md->data_type == MD_DATA_TYPE_FOREIGN) {
        // kern_free(md->data);
    }
    // kern_free(md);
}

void
md_init()
{
    list_create(&memdisks, offsetof(memdisk_t,L_memdisks), FALSE);
}

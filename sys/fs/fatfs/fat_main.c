/*
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
#include <sys/bio.h>
#include <sys/vfs.h>
#include <fs/fatfs/fatfs.h>

vfs_init_t  fatfs_init;


static vfs_mount_t     fat_mount;
static vfs_unmount_t   fat_unmount;
static vfs_getroot_t   fat_getroot;
static vfs_sync_t      fat_sync;

static vfs_ops_t fatfs_ops = {
    fat_mount,
    fat_unmount,
    fat_getroot,
    fat_sync
};

static fatfs_inode_t rootinode;


void
fatfs_init()
{
    vfs_register("fatfs", &fatfs_ops);
}

int read_fat(fatfs_t *fatfs, int i);
int read_root(fatfs_t *fatfs, int i);

int
fat_mount(vfs_t *fs)
{
    int err;
    DEBUGF("reading super block");
    iobuf_t *bp = bio_read(fs->vfs_mdev, 0, 512);
    if ( ISSET(bp->flags,BIO_ERROR) ) {
        DEBUGF("cannot read superblock");
        bio_release(bp);
        return -bp->errno;
    }
    fatfs_sblock_t *sblock = bp->addr;

    if (sblock->media != 0xf0) {
        DEBUGF("only FAT12 on 1440kB floppy is supported");
        bio_release(bp);
        return -ENOTSUP;
    }
    if (sblock->clusize != 1) {
        DEBUGF("%u sectors per cluster not supported", sblock->clusize);
        bio_release(bp);
        return -ENOTSUP;
    }

    fatfs_t *fatfs = kmem_alloc(sizeof(fatfs_t), KM_SLEEP);
    fs->vfs_private = fatfs;
    fatfs->vfs = fs;
    fatfs->tables = sblock->tables;
    fatfs->tablesize = FAT_GET_TABLESIZE(sblock);
    fatfs->secsize = 512;
    fatfs->clusize  = sblock->clusize;
    fatfs->blkno_fat[0] = FAT_GET_RESERVED(sblock);
    fatfs->blkno_fat[1] = fatfs->tablesize + fatfs->blkno_fat[0];
    fatfs->blkno_root = FAT_GET_RESERVED(sblock)
            + fatfs->tablesize * fatfs->tables;
    fatfs->blkno_data = fatfs->blkno_root + (FAT_GET_MAXROOT(sblock)*32)/512;

    bio_release(bp);
    fatfs->clubsize = fatfs->secsize * fatfs->clusize;

    DEBUGF("FAT: diskmap FAT1=%u FAT2=%u ROOT=%u DATA=%u CS=%u",
        fatfs->blkno_fat[0], fatfs->blkno_fat[1], fatfs->blkno_root,
        fatfs->blkno_data, fatfs->clubsize);

    for (int i = 0; i < 2 && i < fatfs->tables; i++) {
        if ( (err = read_fat(fatfs, i)) ) {
            kmem_free(fatfs);
            for (int j = 0; j < i; j++) {
                kmem_free(fatfs->fat[j]);
            }
            return err;
        }
    }

    fatfs->clu_free = FAT12_CLU_FREE;
    fatfs->clu_used = FAT12_CLU_USED;
    fatfs->clu_bad  = FAT12_CLU_BAD;
    fatfs->clu_last = FAT12_CLU_LAST;
    fatfs->dev = fs->vfs_mdev;
    fatfs_inode_prepare(fatfs, &rootinode, FATFS_ROOT);
    rootinode.clustart = fatfs->blkno_root;
    fatfs->root = fatfs_getvnode(&rootinode);

    return 0;
}


int
fat_unmount(vfs_t *fs)
{
    return -ENOTSUP;
}

vnode_t *
fat_getroot(vfs_t *fs)
{
    fatfs_t *fatfs = fs->vfs_private;
    vref(fatfs->root);
    return fatfs->root;
}

void
fat_sync(vfs_t *fs)
{
}

int
read_fat(fatfs_t *fatfs, int x)
{
    DEBUGF("Trying read FAT%u", x+1);
    fatfs->fat[x] = kmem_alloc(fatfs->tablesize*512, KM_SLEEP);
#if 0
    for (int i = 0; i < fatfs->tablesize; i++) {
        iobuf_t *bp = bio_read(fatfs->vfs->vfs_mdev, fatfs->blkno_fat[x] + i, 512);
        if (ISSET(bp->flags,BIO_ERROR)) {
            kmem_free(fatfs->fat[x]);
            bio_release(bp);
            return bp->errno;
        }
        mem_cpy(fatfs->fat[x] + i*512, bp->addr, 512);
        bio_release(bp);
    }
#else
    iobuf_t *bp = bio_read(fatfs->vfs->vfs_mdev, fatfs->blkno_fat[x],fatfs->tablesize*512);
    if (ISSET(bp->flags, BIO_ERROR)) {
        kmem_free(fatfs->fat[x]);
        bio_release(bp);
        return bp->errno;
    }
    mem_cpy(fatfs->fat[x], bp->addr, 512*fatfs->tablesize);
    bio_release(bp);
#endif
    return 0;
}


#define SDEBUG(fmt, ap...) do { DEBUGF(fmt, ## ap); ssleep(1); } while(0)

void *
fatfs_clu_alloc(fatfs_t *fatfs)
{
    return kmem_alloc(fatfs->clubsize, KM_SLEEP);
}

int
fatfs_clu_read(fatfs_t *fatfs, int clu, fatfs_inode_t *inode)
{
    int err = 0;
    if (inode->clunum == clu) return 0;
    clu-=2;
    iobuf_t *bp = bio_read(fatfs->dev, fatfs->blkno_data + clu, 512);
    if (ISSET(bp->flags,BIO_ERROR)) {
        err = -bp->errno;
    } else {
        mem_cpy(inode->clubuf, bp->addr, 512);
    }
    bio_release(bp);
    return 0;
}

void
fatfs_clu_free(fatfs_t *fatfs, void *ptr)
{
    kmem_free(ptr);
}


// [AAABBB][CCCDDD]
uint
fatfs_fat_next(fatfs_t *fatfs, uint i)
{
    int n = (3*i) / 2;
    uint16_t fat = *(uint16_t*)&((fatfs->fat[0])[n]);
    uint16_t a,b;

    a = (fat & 0xfff);
    b = (fat & 0xfff0) >>4;
//      DEBUGF("trying to get %u entry, n=%u %x=(%u,%u)",
//          i, n, fat, a, b);
    if ( (3*i) & 1) {
        return b;
    } else {
        return a;
    }
}


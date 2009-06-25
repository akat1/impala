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
#include <sys/vfs.h>
#include <sys/bio.h>
#include <fs/fatfs/fatfs.h>

int load(fatfs_inode_t *, int i);
int read_root(fatfs_inode_t *);

enum {
    LFN_CACHE_MAX   = 10,
};

typedef struct lfn_cache lfn_cache_t;
struct lfn_cache {
    int             used;
    fatfs_lname_t   chunks[LFN_CACHE_MAX];
};

void lfn_cache_reset(lfn_cache_t *cache);
void lfn_cache_insert(lfn_cache_t *cache, fatfs_dentry_t *d);
int lfn_cache_parse(lfn_cache_t *cache, char *output);

#define NEW_DIRENT() kmem_zalloc(sizeof(fatfs_dirent_t), KM_SLEEP)

/*============================================================================
 * Operacje na katalogach.
 */

int
fatfs_dirent_lookup(fatfs_inode_t *inode, fatfs_inode_t **r, const char *name)
{
    if (fatfs_dirent_read(inode)) return -1;

    fatfs_dirent_t *entry = NULL;
    while ( (entry = list_next(&inode->un.dir.dirents, entry)) ) {
        if ( str_eq(name,entry->name) ) {
            if (entry->inode == NULL) {
                fatfs_inode_create(inode->fatfs, entry);
            }
            *r = entry->inode;
            return 0;
        }
    }

    return -1;
}

#define SDEBUGF(fmt, ap...) do { DEBUGF(fmt, ## ap); /*ssleep(1);*/ } while(0)
int
fatfs_dirent_read(fatfs_inode_t *inode)
{
    if (inode->flags & FATFS_DIR_LOADED) return 0;
//     if (inode->type == FATFS_ROOT) return read_root(inode);
    fatfs_t *fatfs = inode->fatfs;
    lfn_cache_t lfn;

    bool isRoot = inode->type == FATFS_ROOT;
    int N = (isRoot)? fatfs->secsize : fatfs->clubsize;
    N /= sizeof(fatfs_dentry_t);
    int i = inode->clustart;

    lfn_cache_reset(&lfn);
//     SDEBUGF("need to load directory from device clu=%u/%u %p", i,N, inode);
//     SDEBUGF("inode=%p vnode=%p fatfs=%p clubuf=%p",
//         inode, inode->vn, inode->fatfs, inode->clubuf);
    do {
//         SDEBUGF("current clu=%i", i);
        if (isRoot) {
//             SDEBUGF("reading sector");
            iobuf_t *bp = bio_read(fatfs->dev, i);
            if (ISSET(bp->flags, BIO_ERROR)) {
                bio_release(bp);
                goto error;
            }
            mem_cpy(inode->clubuf, bp->addr, fatfs->secsize);
            bio_release(bp);
            i++;
        } else { //SDEBUGF("reading cluster");
        if (fatfs_clu_read(fatfs, i, inode)) {
            goto error;
        }}
        fatfs_dentry_t *dents = inode->clubuf;
//         SDEBUGF("Interpreting content...");
        for (int j = 0; j < N; j++) {
            if (dents[j].attr == FATFS_LONGNAME) {
                lfn_cache_insert(&lfn, &dents[j]);
                continue;
            }
            if (dents[j].attr == 0 || ISSET(dents[j].attr,FATFS_SKIP))
                continue;
//             SDEBUGF("entry");
            fatfs_dirent_t *entry = NEW_DIRENT();
            if (lfn_cache_parse(&lfn, entry->name)) {
                char *ptr = entry->name;
                for (int k = 0; dents[j].name[k] != ' ' && k < 8; k++, ptr++) {
                    *ptr = dents[j].name[k];
                }
                if (dents[j].ext[0] != ' ') {
                    *ptr = '.';
                    ptr++;
                }
                for (int k = 0; dents[j].ext[k] != ' ' && k < 3; k++, ptr++) {
                    *ptr = dents[j].ext[k];
                }
                *ptr = 0;
            }
            entry->attr = dents[j].attr;
            entry->size = FAT_D_GET_SIZE(&dents[j]);
            entry->clustart = FAT_D_GET_INDEX(&dents[j]);
            entry->inode = NULL;
//             DEBUGF("+%s %u %u",entry->name, entry->clustart, entry->size);
            list_insert_tail(&inode->un.dir.dirents, entry);
            lfn_cache_reset(&lfn);
        }
    } while ( (isRoot)? i < fatfs->tablesize :  FATFS_UNTIL_EOF(fatfs,i) );
    inode->flags |= FATFS_DIR_LOADED;
    return 0;
error:
    return -1;
}

/*============================================================================
 * Pomocnicze procedury
 */

int
read_root(fatfs_inode_t *inode)
{
    DEBUGF("need to load ROOT directory from device");
    lfn_cache_t lfn;
    fatfs_t *fatfs = inode->fatfs;
    lfn_cache_reset(&lfn);
    for (int i = 0; i < fatfs->tablesize; i++) {
        iobuf_t *bp = bio_read(fatfs->dev, fatfs->blkno_root+i);
        if (ISSET(bp->flags, BIO_ERROR)) {
            bio_release(bp);
            goto error;
        }
        fatfs_dentry_t *dents = bp->addr;
        for (int j = 0; j < fatfs->secsize/sizeof(fatfs_dentry_t); j++) {
            if (dents[j].attr == FATFS_LONGNAME) {
                lfn_cache_insert(&lfn, &dents[j]);
                continue;
            }
            if (dents[j].attr == 0 || ISSET(dents[j].attr,FATFS_SKIP))
                continue;

            fatfs_dirent_t *entry = NEW_DIRENT();
            if (lfn_cache_parse(&lfn, entry->name)) {
                char *ptr = entry->name;
                for (int k = 0; dents[j].name[k] != ' ' && k < 8; k++, ptr++) {
                    *ptr = dents[j].name[k];
                }
                if (dents[j].ext[0] != ' ') {
                    *ptr = '.';
                    ptr++;
                }
                for (int k = 0; dents[j].ext[k] != ' ' && k < 3; k++, ptr++) {
                    *ptr = dents[j].ext[k];
                }
                *ptr = 0;
            }
            entry->attr = dents[j].attr;
            entry->size = FAT_D_GET_SIZE(&dents[j]);
            entry->clustart = FAT_D_GET_INDEX(&dents[j]);
            list_insert_tail(&inode->un.dir.dirents, entry);
            lfn_cache_reset(&lfn);
        }
        bio_release(bp);
    }
    inode->flags |= FATFS_DIR_LOADED;
    return 0;
error:
    DEBUGF("I/O error");
    ///@todo zwolniæ zasoby
    return -1;
}

int
load(fatfs_inode_t *inode, int i)
{
    int err = 0;
    fatfs_t *fatfs = inode->fatfs;
    if (inode->type == FATFS_ROOT) {
        if (inode->fatfs->tablesize <= i) return -1;
        if (inode->clunum != i) {
            inode->clunum = i;
            mem_zero(inode->clubuf, fatfs->secsize*fatfs->clusize);
            iobuf_t *bp = bio_read(fatfs->dev, fatfs->blkno_root + i);
            if (ISSET(bp->flags, BIO_ERROR)) {
                err = -1;
            } else {
                mem_cpy(inode->clubuf, bp->addr, fatfs->secsize);
            }
            bio_release(bp);
        }
    } else {
        err = -1;
    }
    return err;
}


void
lfn_cache_reset(lfn_cache_t *cache)
{
    mem_zero(cache, sizeof(*cache));
}


void
lfn_cache_insert(lfn_cache_t *cache, fatfs_dentry_t *d)
{
    fatfs_lname_t *lfn = (fatfs_lname_t*) d;
    if (lfn->n & FATFS_LONGNAME_LAST) {
        UNSET(lfn->n, FATFS_LONGNAME_LAST);
        cache->used = lfn->n;
    }
    if (lfn->n == 0) return;
    lfn->n--;
    if (LFN_CACHE_MAX <= lfn->n) return;
    mem_cpy(&cache->chunks[lfn->n], lfn, 32);

}

int
lfn_cache_parse(lfn_cache_t *lfn, char *output)
{
//     const char *p = output;

#define NOT_END(j,name)\
    (lfn->chunks[i].name[2*j] != 0x00 && \
    lfn->chunks[i].name[2*j]  != 0xff)

    int err = -1;
    for (int i = 0; i < lfn->used; i++) {
        err = 0;
        for (int j = 0; j < 5 && NOT_END(j,name1); j++, output++) {
            *output = lfn->chunks[i].name1[2*j];
        }
        for (int j = 0; j < 6 && NOT_END(j,name2); j++, output++) {
            *output = lfn->chunks[i].name2[2*j];
        }
        for (int j = 0; j < 2 && NOT_END(j,name3); j++, output++) {
            *output = lfn->chunks[i].name3[2*j];
        }
        *output = 0;
    }

#undef NOT_END
    return err;
}


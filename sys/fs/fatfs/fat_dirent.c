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

enum {
    BSIZE   = 512
};

enum VFAT_NAMES {
    VFAT_NAME_PARTS = 0x10
};

typedef struct vfatlname vfatlname_t;

struct vfatlname {
    int             ready;
    fatfs_lname_t   parts[VFAT_NAME_PARTS];
};

static void vfatlname_reset(vfatlname_t *);
static void vfatlname_insert(vfatlname_t *, const fatfs_dentry_t *);
static void vfatlname_copy(vfatlname_t *, fatfs_dirent_t *,
    const fatfs_dentry_t *);
static void vfatlname_copy83(vfatlname_t *, fatfs_dirent_t *,
    const fatfs_dentry_t *);
static void vfatlname_copylong(vfatlname_t *, fatfs_dirent_t *);

static int load_from_root(fatfs_dir_t *dir);
static int load_from_dir(fatfs_dir_t *dir);
static void check_entry(const fatfs_dentry_t *, vfatlname_t *, fatfs_dir_t *);
static int rewrite_whole_dir(fatfs_dir_t *dir);

#define UTF16CUT(u16) ((u16) & 0x0f)


/*============================================================================
 * Operacje na katalogach.
 */

/**
 * szuka wêz³a w katalogu.
 * @param dir deksryptor katalogu
 * @param name nazwa wpisu
 *
 * Poniewa¿ katalogi zapamiêtuj± wêz³y swoich wpisów to nale¿y zwiêkszyæ
 * odpowiednio licznik odniesieñ. Jeden jest dla klienta procedury, a drugi
 * dla katalogu.
 */
fatfs_node_t *
fatfs_dir_lookup(fatfs_dir_t *dir, const char *name)
{
    fatfs_dirent_t *dirent = list_find(&dir->dirents, str_eq, name);
    if (!dirent) return NULL;
    if (dirent->node) {
        // skoro wêze³ istnieje, to ma ju¿ nasz± 'referencjê' w liczniku
        // wiêc robimy tylko referencje dla klienta
        fatfs_node_ref(dirent->node);
        return dirent->node;
    }
    dirent->node = fatfs_node_alloc(dir->node->fatfs, dirent->type);
    dirent->node->firstclu = dirent->firstclu;
    dirent->node->size = dirent->size;
    dirent->node->dirent = dirent;
    // dopiero co utworzony wêze³, ma jedn± referencjê ustawion± z procedury
    // alloc, traktujemy to jako referencjê wpisu w katalogu, wiêc musimy
    // jeszcze zrobiæ referencjê dla klienta procedury
    fatfs_node_ref(dirent->node);
    return dirent->node;
}

int
fatfs_dir_getdents(fatfs_dir_t *dir, dirent_t *dents, int first, int n)
{
    int r;
    fatfs_dirent_t *dirent = list_get_n(&dir->dirents, first);
    if (dirent == NULL) return 0;
    for (r = 0; r < n && dirent; r++) {
        str_ncpy(dents[r].d_name, dirent->name, MAX_NAME);
        dents[r].d_ino = dirent->firstclu;
        dirent = list_next(&dir->dirents, dirent);
    }
    return r*sizeof(dirent_t);
}

fatfs_node_t *
fatfs_dir_create(fatfs_dir_t *dir, const char *name, int type)
{
    KASSERT(type == FATFS_DIR || type == FATFS_REG);
    fatfs_node_t *node = fatfs_node_alloc(dir->node->fatfs, type);
    fatfs_dirent_t *dirent = kmem_zalloc( sizeof(*dirent), KM_SLEEP );
    str_ncpy(node->name, name, sizeof(node->name));
    str_ncpy(dirent->name, name, sizeof(dirent->name));
    dirent->node = node;
    node->dirent = dirent;

    dirent->type = type;
    dirent->attr = (type == FATFS_REG)? 0 : FATFS_SUBDIR;
    dirent->size = 0;
    dirent->firstclu = 0;
    node->size = 0;
    node->firstclu = 0;
    list_insert_tail(&dir->dirents, dirent);
    rewrite_whole_dir(dir);
    return node;
}

void
fatfs_dir_remove(fatfs_dir_t *dir, const char *path)
{
}


int
fatfs_dir_load(fatfs_node_t *node)
{
    int err;
    KASSERT(node->type == FATFS_ROOT || node->type == FATFS_DIR);
    KASSERT(node->dir == NULL);
    node->dir = kmem_zalloc(sizeof(*node->dir), KM_SLEEP);
    node->dir->node = node;
    LIST_CREATE(&node->dir->dirents, fatfs_dirent_t, L_dirents, FALSE);

    if (node->type == FATFS_ROOT) {
        err = load_from_root(node->dir);
    } else {
        err = load_from_dir(node->dir);
    }
    return err;
}

void
fatfs_dir_free(fatfs_dir_t *dir)
{
    fatfs_dirent_t *dirent = NULL;
    while ( (dirent = list_extract_first(&dir->dirents)) ) {
        if (dirent->node) {
            dirent->node->dirent = 0;
            fatfs_node_rel(dirent->node);
        }
        kmem_free(dirent);
    }
    kmem_free(dir);
}


/*============================================================================
 * Pomocnicze procedury
 */

int
rewrite_whole_dir(fatfs_dir_t *dir)
{
    fatfs_dirent_t *dirent = list_head(&dir->dirents);
    fatfs_t *fatfs = dirent->node->fatfs;
    fatfs_dentry_t dentry;
    ssize_t size = sizeof(dentry)*list_length(&dir->dirents);
    blkno_t blk;
    iobuf_t *bp;
    off_t off = 0;
    size_t bcount = 1;
    size = (size + fatfs->clusize-1) / fatfs->clusize;
    size *= fatfs->clusize;
    while ( (blk = fatfs_bmap(dir->node, off, size, &bcount)) != -1 ) {
        size_t lsize = bcount * fatfs->clubsize;
        size -= lsize;
        off += lsize;
        bp = bio_getblk(fatfs->dev, blk, lsize);
        mem_zero(bp->addr, lsize);
        bio_write(bp);
        if ( ISSET(bp->flags, BIO_ERROR) ) {
            int err = bp->errno;
            bio_release(bp);
            return -err;
        }
        bio_release(bp);
    }
    return 0;
}



int
load_from_root(fatfs_dir_t *dir)
{
    const fatfs_dentry_t *table;
    fatfs_t *fatfs = dir->node->fatfs;
    iobuf_t *bp;
    vfatlname_t vfc; //XXX
    bp = bio_read(fatfs->dev, fatfs->blkno_root, fatfs->maxroot*32);
    if ( ISSET(bp->flags, BIO_ERROR) ) {
        bio_release(bp);
        return -bp->errno;
    }
    vfatlname_reset(&vfc);
    table = bp->addr;
    for (int i = 0; i < fatfs->maxroot; i++) {
        check_entry(&table[i], &vfc, dir);
    }
    bio_release(bp);
    return 0;
}

int
load_from_dir(fatfs_dir_t *dir)
{
    iobuf_t *bp;
    fatfs_t *fatfs = dir->node->fatfs;
    size_t cont = 1;
    off_t off = 0;
    blkno_t blk;
    vfatlname_t vln;
    vfatlname_reset(&vln);
    while ( (blk = fatfs_bmap(dir->node, off, 0, &cont)) != -1 ) {
        size_t size = cont*fatfs->clusize;
        off += size;
        const fatfs_dentry_t *table;
        bp = bio_read(fatfs->dev, blk, size);
        if ( ISSET(bp->flags, BIO_ERROR) ) {
            bio_release(bp);
            return -bp->errno;
        }
        bio_release(bp);
        table = bp->addr;
        for (int i = 0; i < size/sizeof(fatfs_dentry_t); i++) {
            check_entry(&table[i], &vln, dir);
        }
    }
    return 0;
}

void
check_entry(const fatfs_dentry_t *dentry, vfatlname_t *vln, fatfs_dir_t *dir)
{  
    if (dentry->attr == FATFS_LONGNAME) {
        vfatlname_insert(vln, dentry);
        return;
    }
    if (dentry->attr == 0 || ISSET(dentry->attr, FATFS_SKIP)) return;
    if ( mem_cmp(dentry->name, ".       ", 8) == 0) return;
    if ( mem_cmp(dentry->name, "..      ", 8) == 0) return;
    
    if (dentry->name[0] != 0xe5) {
        fatfs_dirent_t *dirent = kmem_zalloc(sizeof(*dirent), KM_SLEEP);
        vfatlname_copy(vln, dirent, dentry);
        dirent->attr = dentry->attr;
        if (dirent->attr & FATFS_SUBDIR) {
            dirent->type = FATFS_DIR;
        } else {
            dirent->type = FATFS_REG;
        }
        dirent->firstclu = FATFS_D_GET_INDEX(dentry);
        dirent->size = FATFS_D_GET_SIZE(dentry);
        list_insert_tail(&dir->dirents, dirent);
    }
    vfatlname_reset(vln);
}

/*============================================================================
 * Obs³uga d³ugich nazw VFAT
 */

void
vfatlname_reset(vfatlname_t *vln)
{
    mem_zero(vln, sizeof(*vln));
}

void
vfatlname_insert(vfatlname_t *vln, const fatfs_dentry_t *d)
{
    fatfs_lname_t *lname = (fatfs_lname_t*) d;
    if (lname->n & FATFS_LONGNAME_LAST) {
        UNSET(lname->n, FATFS_LONGNAME_LAST);
        vln->ready = lname->n;
    }
    if (lname->n-- == 0) return;
    if (lname->n < VFAT_NAME_PARTS) {
        mem_cpy(&vln->parts[lname->n], lname, sizeof(*lname));
    }
}

void
vfatlname_copy(vfatlname_t *vln, fatfs_dirent_t *dirent,
    const fatfs_dentry_t *d)
{
    if (vln->ready) {
        vfatlname_copylong(vln, dirent);
    } else {
        vfatlname_copy83(vln, dirent, d);
    }
}

void
vfatlname_copy83(vfatlname_t *vln, fatfs_dirent_t *dirent,
    const fatfs_dentry_t *d)
{
#define ugly_tolower(c) ( ('A' <= (c) && (c) <= 'Z')? (c) - ('A'-'a') : (c))
    str_cpy(dirent->name, (const char *)d->name);
    for (int i = 0; i < 8; i++)
        if (dirent->name[i] == ' ') dirent->name[i] = 0;
    if (d->ext[0] != ' ') {
        str_cat(dirent->name, ".");
        str_cat(dirent->name, (const char *)d->ext);
        for (int i = 0; i < 14; i++)
            if (dirent->name[i] == ' ') dirent->name[i] = 0;
    }
    for (int i = 0; dirent->name[i]; i++)
        dirent->name[i] = ugly_tolower(dirent->name[i]);
#undef ugly_tolower

}


void
vfatlname_copylong(vfatlname_t *vln, fatfs_dirent_t *dirent)
{
#define NOT_END(j,name)\
    (vln->parts[i].name[2*j] != 0x00 && \
     vln->parts[i].name[2*j] != 0xff)
    char *output = dirent->name;
    for (int i = 0; i < vln->ready; i++) {
        for (int j = 0; j < 5 && NOT_END(j,name1); j++, output++) {
            *output = vln->parts[i].name1[2*j];
        }
        for (int j = 0; j < 6 && NOT_END(j,name2); j++, output++) {
            *output = vln->parts[i].name2[2*j];
        }
        for (int j = 0; j < 2 && NOT_END(j,name3); j++, output++) {
            *output = vln->parts[i].name3[2*j];
        }
        *output = 0;
    }
#undef NOT_END
}

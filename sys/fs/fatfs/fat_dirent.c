/*
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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
    VFAT_NAME_PARTS = 20
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
int vfatlname_generate(vfatlname_t *, const char *name, uint8_t sum);
void vfatlname_fill(vfatlname_t *, fatfs_dentry_t *);

static int load_from_root(fatfs_dir_t *dir);
static int load_from_dir(fatfs_dir_t *dir);
static void check_dentry(const fatfs_dentry_t *, vfatlname_t *, fatfs_dir_t *);
static int fill_dentry(fatfs_dentry_t *, const fatfs_dirent_t *, int,int );
static int rewrite_whole_dir(fatfs_dir_t *dir);
static int rewrite_whole_root(fatfs_dir_t *dir);

#define UTF16CUT(u16) ((u16) & 0x0f)


/*============================================================================
 * Operacje na katalogach.
 */

/**
 * szuka węzła w katalogu.
 * @param dir deksryptor katalogu
 * @param name nazwa wpisu
 *
 * Ponieważ katalogi zapamiętują węzły swoich wpisów to należy zwiększyć
 * odpowiednio licznik odniesień. Jeden jest dla klienta procedury, a drugi
 * dla katalogu.
 */
fatfs_node_t *
fatfs_dir_lookup(fatfs_dir_t *dir, const char *name)
{
    fatfs_dirent_t *dirent = list_find(&dir->dirents, streq, name);
    fatfs_t *fatfs = dir->node->fatfs;

    if (!dirent) return NULL;
    if (dirent->node) {
        // skoro węzeł istnieje, to ma już naszą 'referencję' w liczniku
        // więc robimy tylko referencje dla klienta
        fatfs_node_ref(dirent->node);
        return dirent->node;
    }
    dirent->node = fatfs_node_alloc(dir->node->fatfs, dirent->type);
    dirent->node->firstclu = dirent->firstclu;
    dirent->node->size = dirent->size;
    dirent->node->csize = (dirent->size + (fatfs->clusize-1))/fatfs->clusize;
    dirent->node->dirent = dirent;
    // dopiero co utworzony węzeł, ma jedną referencję ustawioną z procedury
    // alloc, traktujemy to jako referencję wpisu w katalogu, więc musimy
    // jeszcze zrobić referencję dla klienta procedury
    fatfs_node_ref(dirent->node);
    KASSERT(dirent->node->refcnt == 2);
    return dirent->node;
}

int
fatfs_dir_getdents(fatfs_dir_t *dir, dirent_t *dents, int first, int n)
{
    int r;
    fatfs_dirent_t *dirent = list_get_n(&dir->dirents, first);
    if (dirent == NULL) return 0;
    for (r = 0; r < n && dirent; r++) {
        strncpy(dents[r].d_name, dirent->name, MAX_NAME);
        dents[r].d_ino = dirent->firstclu;
        dirent = list_next(&dir->dirents, dirent);
    }
    return r*sizeof(dirent_t);
}

fatfs_node_t *
fatfs_dir_create(fatfs_dir_t *dir, const char *name, int type, blkno_t clu)
{
    KASSERT(type == FATFS_DIR || type == FATFS_REG);
    fatfs_node_t *node = fatfs_node_alloc(dir->node->fatfs, type);
    fatfs_dirent_t *dirent = kmem_zalloc( sizeof(*dirent), KM_SLEEP );
    strncpy(node->name, name, sizeof(node->name));
    strncpy(dirent->name, name, sizeof(dirent->name));
    dirent->node = node;
    dirent->dirnode = dir->node;
    node->dirent = dirent;

    dirent->type = type;
    dirent->attr = (type == FATFS_REG)? 0 : FATFS_SUBDIR;
    dirent->size = 0;
    dirent->firstclu = clu;
    node->size = 0;
    node->firstclu = clu;
    list_insert_tail(&dir->dirents, dirent);
    fatfs_dir_sync(dir);
    fatfs_node_ref(node);
    return node;
}

void
fatfs_dir_sync(fatfs_dir_t *dir)
{
    if (dir->node->type == FATFS_ROOT) {
        rewrite_whole_root(dir);
    } else {
        rewrite_whole_dir(dir);
    }
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
 * Obsługa zapisu i odczytu katalogów z nośnika
 */

int
rewrite_whole_dir(fatfs_dir_t *dir)
{
    int err;
    uio_t u;
    iovec_t iov;
    fatfs_dirent_t *dirent;
    size_t size = list_length(&dir->dirents);

    err = fatfs_node_truncate(dir->node, 512);
    if (err) return err;

    fatfs_dentry_t *table = kmem_zalloc(512, KM_SLEEP);
    if (table == NULL) {
        DEBUGF("cannot allocate buffer for %u entries", size);
        return -ENOMEM;
    }
    u.space = UIO_SYSSPACE;
    u.oper = UIO_WRITE;
    u.iovs = &iov;
    u.iovcnt = 1;
    u.size = 512;
    u.offset = 0;
    u.resid = 512;
    u.completed = 0;
    iov.iov_len = 512;
    iov.iov_base = table;

    dirent = list_head(&dir->dirents);
    for (int i = 0; i < 512/32 && dirent; ) {
        i += fill_dentry(&table[i], dirent, 512/32 - i, i);
        dirent = list_next(&dir->dirents, dirent);
    }
    fatfs_node_write(dir->node, &u, 0);
    kmem_free(table);

    return 0;
}

int
rewrite_whole_root(fatfs_dir_t *dir)
{
    fatfs_t *fatfs = dir->node->fatfs;
    fatfs_dirent_t *dirent;
    fatfs_dentry_t *dentry;
    iobuf_t *bp;

    bp = bio_getblk(fatfs->dev, fatfs->blkno_root, fatfs->maxroot*32);
    memzero(bp->addr, bp->size);
    dentry = bp->addr;
    dirent = list_head(&dir->dirents);
    for (int i = 0; i < fatfs->maxroot && dirent; ) {
        if (!streq(dirent->name, ".") && !streq(dirent->name,"..")) {
            i += fill_dentry(&dentry[i], dirent, fatfs->maxroot - i, i);
        }
        dirent = list_next(&dir->dirents, dirent);
    }
    bio_write(bp);
    bio_wait(bp);
    bio_release(bp);

    return 0;
}

int
fill_dentry(fatfs_dentry_t *dentry, const fatfs_dirent_t *dirent,
    int left, int n)
{
    fatfs_dentry_t copy;
    uint8_t sum = 0;
    char *helper = (char*) copy.name;
    vfatlname_t vln;

    memzero(&copy, sizeof(copy));
    snprintf(helper, 9, "E%07u", n);
    memset(copy.ext, ' ', 3);

    for (int i = 0; i < 11; i++) {
        sum = (((sum & 1) << 7) | ((sum & 0xFE) >> 1)) + helper[i];
    }

    copy.attr =  (dirent->type == FATFS_REG)? 0 : FATFS_SUBDIR;
    FATFS_D_SET_INDEX(&copy, dirent->firstclu);
    FATFS_D_SET_SIZE(&copy, dirent->size);

    vfatlname_reset(&vln);
    int x = vfatlname_generate(&vln, dirent->name, sum);
    if ( x < left ) {
        vfatlname_fill(&vln, dentry);
    } else x = 0;
    memcpy(dentry+x, &copy, sizeof(copy));
    return 1+x;
}

int
load_from_root(fatfs_dir_t *dir)
{
    const fatfs_dentry_t *table;
    fatfs_t *fatfs = dir->node->fatfs;
    iobuf_t *bp;
    vfatlname_t vfc;
    fatfs_dirent_t *dirent;

    dirent  = kmem_zalloc(sizeof(*dirent), KM_SLEEP);
    strcpy(dirent->name, ".");
    dirent->firstclu = dir->node->firstclu;
    dirent->size = 0;
    dirent->dirnode = dir->node;
    dirent->node = dir->node;
    list_insert_tail(&dir->dirents, dirent);

    dirent = kmem_zalloc(sizeof(*dirent), KM_SLEEP);
    strcpy(dirent->name, "..");
    dirent->firstclu = dir->node->firstclu;
    dirent->size = 0;
    dirent->dirnode = dir->node;
    dirent->node = dir->node;
    list_insert_tail(&dir->dirents, dirent);

    bp = bio_read(fatfs->dev, fatfs->blkno_root, fatfs->maxroot*32);
    if ( ISSET(bp->flags, BIO_ERROR) ) {
        bio_release(bp);
        return -bp->errno;
    }
    vfatlname_reset(&vfc);
    table = bp->addr;
    for (int i = 0; i < fatfs->maxroot; i++) {
        check_dentry(&table[i], &vfc, dir);
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
            check_dentry(&table[i], &vln, dir);
        }
    }

    dir->node->csize = (off + (fatfs->clusize-1))/fatfs->clusize;
    return 0;
}

void
check_dentry(const fatfs_dentry_t *dentry, vfatlname_t *vln, fatfs_dir_t *dir)
{
    fatfs_t *fatfs = dir->node->fatfs;
    if (dentry->attr == FATFS_LONGNAME) {
        vfatlname_insert(vln, dentry);
        return;
    }
    if (dentry->name[0] == 0) return;
    if (ISSET(dentry->attr, FATFS_SKIP)) return;
    if ( memcmp(dentry->name, ".       ", 8) == 0) {
        fatfs_dirent_t *dirent = kmem_zalloc(sizeof(*dirent), KM_SLEEP);
        strcpy(dirent->name, ".");
        dirent->firstclu = dir->node->firstclu;
        dirent->size = 0;
        dirent->dirnode = dir->node;
        dirent->node = dir->node;
        list_insert_tail(&dir->dirents, dirent);
    } else
    if ( memcmp(dentry->name, "..      ", 8) == 0) {
        fatfs_dirent_t *dirent = kmem_zalloc(sizeof(*dirent), KM_SLEEP);
        strcpy(dirent->name, "..");
        dirent->firstclu = dir->node->dirent->dirnode->firstclu;
        dirent->size = 0;
        dirent->dirnode = dir->node;
        dirent->node = dir->node->dirent->dirnode;
        list_insert_tail(&dir->dirents, dirent);
    } else
    if (dentry->name[0] != 0xe5) {
        fatfs_dirent_t *dirent = kmem_zalloc(sizeof(*dirent), KM_SLEEP);
        fatfs_node_t *node;
        vfatlname_copy(vln, dirent, dentry);
        dirent->attr = dentry->attr;
        if (dirent->attr & FATFS_SUBDIR) {
            dirent->type = FATFS_DIR;
        } else {
            dirent->type = FATFS_REG;
        }
        dirent->firstclu = FATFS_D_GET_INDEX(dentry);
        dirent->size = FATFS_D_GET_SIZE(dentry);
        dirent->dirnode = dir->node;
        node = fatfs_node_alloc(dir->node->fatfs, dirent->type);
        node->firstclu = dirent->firstclu;
        node->size = dirent->size;
        node->csize = (dirent->size + (fatfs->clusize-1))/fatfs->clusize;
        node->dirent = dirent;
        dirent->node = node;
        list_insert_tail(&dir->dirents, dirent);
    } else {
        // !?
    }
    vfatlname_reset(vln);
}

/*============================================================================
 * Obsługa długich nazw
 */

void
vfatlname_reset(vfatlname_t *vln)
{
    memzero(vln, sizeof(*vln));
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
        memcpy(&vln->parts[lname->n], lname, sizeof(*lname));
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
    strncpy(dirent->name, (const char *)d->name, 8);
    for (int i = 0; i < 8; i++)
        if (dirent->name[i] == ' ') dirent->name[i] = 0;
    if (d->ext[0] != ' ') {
        strcat(dirent->name, ".");
        strcat(dirent->name, (const char *)d->ext);
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

int
vfatlname_generate(vfatlname_t *vln, const char *name, uint8_t sum)
{
    int n =  (strlen(name)+12)/13;
    KASSERT(n < VFAT_NAME_PARTS);
    for (int i = 0; i < n && *name; i++) {
        char *helper;
        int j;
        vln->parts[i].n = i+1;
        vln->parts[i].attr = FATFS_LONGNAME;
        vln->parts[i].clu[0] = 0;
        vln->parts[i].clu[1] = 0;
        vln->parts[i].checksum = sum;

        memset(vln->parts[i].name1, 0xff, 10);
        memset(vln->parts[i].name2, 0xff, 12);
        memset(vln->parts[i].name3, 0xff,  4);

        helper = (char*)vln->parts[i].name1;
        for (j = 0; j < 5 && *name; j++, name++) {
            helper[2*j] = *name;
            helper[2*j+1] = 0;
        }
        if (j==5) {
            helper = (char*)vln->parts[i].name2;
            j = 0;
        }
        if (!*name) {
            helper[2*j] = 0x00;
            helper[2*j+1] = 0x00;
            break;
        }
        for (j = 0; j < 6 && *name; j++, name++) {
            helper[2*j+1] = 0;
            helper[2*j] = *name;
        }
        if (j==6) {
            helper = (char*)vln->parts[i].name3;
            j = 0;
        }
        if (!*name) {
            helper[2*j] = 0x00;
            helper[2*j+1] = 0x00;
            break;
        }
        for (j = 0; j < 2 && *name; j++, name++) {
            helper[2*j+1] = 0;
            helper[2*j] = *name;
        }
        if (!*name) {
            helper[2*j] = 0x00;
            helper[2*j+1] = 0x00;
            break;
        }
    }
    vln->parts[n-1].n |=  FATFS_LONGNAME_LAST;
    vln->ready = n;
    return n;
}

void
vfatlname_fill(vfatlname_t *vln, fatfs_dentry_t *dentry)
{
    memcpy(dentry, vln->parts, vln->ready * sizeof(fatfs_lname_t));
}


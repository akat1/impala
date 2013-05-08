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

/** @file 
 * Główny plik obsługi systemu plików MS FAT.
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/bio.h>
#include <sys/vfs.h>
#include <fs/fatfs/fatfs.h>

enum {
    BSIZE   = 512
};

vfs_init_t  fatfs_init;
static vfs_mount_t     fatfs_mount;
static vfs_unmount_t   fatfs_unmount;
static vfs_getroot_t   fatfs_getroot;
vfs_sync_t      fatfs_sync;

static vfs_ops_t fatfs_ops = {
    fatfs_mount,
    fatfs_unmount,
    fatfs_getroot,
    fatfs_sync
};

int fatfs_check_sblock(const fatfs_sblock_t *fs);
fatfs_t *fatfs_createfs(const fatfs_sblock_t *fs);
int fatfs_read_fat(fatfs_t *fatfs);

blkno_t fat12_get(const uint8_t *, int);
void fat12_set(uint8_t *, int, blkno_t);

/*************************************************************************
 * Procedury obsługi VFS.
 */


/// rejestruje FATFS
void
fatfs_init()
{
    vfs_register("fatfs", &fatfs_ops);
}

/// montuje FATFS
int
fatfs_mount(vfs_t *vfs)
{
    
    devd_t *dev = vfs->vfs_mdev;
    iobuf_t *bp;
    fatfs_t *fatfs = NULL;
    fatfs_node_t *rootnode;
    int err;
    bp = bio_read(dev, 0, BSIZE);
    if ( ISSET(bp->flags, BIO_ERROR) ) {
        DEBUGF("read super block failed");
        bio_release(bp);
        return -bp->errno;
    }
    if (!fatfs_check_sblock(bp->addr))
        fatfs = fatfs_createfs(bp->addr);
    bio_release(bp);
    if (fatfs == NULL) return -EINVAL;
    fatfs->dev = vfs->vfs_mdev;
    fatfs->vfs = vfs;
    vfs->vfs_private = fatfs;
    err = fatfs_read_fat(fatfs);
    if (err) {
        kmem_free(fatfs);
        return err;
    }
    fatfs_space_scan(fatfs);

    rootnode = fatfs_node_alloc(fatfs, FATFS_ROOT);
    fatfs->root = fatfs_node_getv(rootnode);
    return 0;
}


int
fatfs_unmount(vfs_t *vfs)
{
    return -ENOTSUP;
}

/// pobiera opis głównego katalogu systemu plików
vnode_t *
fatfs_getroot(vfs_t *vfs)
{
    fatfs_t *fatfs = vfs->vfs_private;
    vref(fatfs->root);
    return fatfs->root;
}

/// synchronizuje system plików z nośnikiem
void
fatfs_sync(vfs_t *vfs)
{
    fatfs_t *fatfs = vfs->vfs_private;
    iobuf_t *bp;

    fatfs_space_repair(fatfs);

    bp = bio_getblk(fatfs->dev, fatfs->blkno_fat, fatfs->fatbsize*BSIZE);
    mem_cpy(bp->addr, fatfs->fat, fatfs->fatbsize*BSIZE);
    bio_write(bp);
    bio_wait(bp);
    if ( ISSET(bp->flags, BIO_ERROR) ) {
        bio_release(bp);
        return;
    }
    bio_release(bp);
    fatfs_space_scan(fatfs);

}

/*************************************************************************
 * Procedury zarządzające miejscem na nośniku.
 */

/**
 *
 */
void
fatfs_space_scan(fatfs_t *fatfs)
{
    blkno_t ffree = 0;
    blkno_t flast = 0;
    size_t elems = (fatfs->blkno_last-fatfs->blkno_data);
    int c = 0;
    for (int i = 0; i < elems; i++) {
        blkno_t clu = fatfs_fatget(fatfs, i);
        if (clu == 0) {
            c++;
            if (ffree==0) {
                ffree = i;
            } else {
                fatfs->fat_set(fatfs->fat, flast, i);
            }
            flast = i;
        }
    }
    fatfs->free_first = ffree;
    fatfs->free_count = c;
}

void
fatfs_space_repair(fatfs_t *fatfs)
{
    if (fatfs->free_count == 0) return;

    blkno_t clu = fatfs->free_first;
    blkno_t nextclu;
    for (int i = 0; i < fatfs->free_count; i++) {
        nextclu = fatfs->fat_get(fatfs->fat, clu);
        fatfs->fat_set(fatfs->fat, clu, 0);
        clu = nextclu;
    }
}


/// przydziela wolne miejsce na nośniku
blkno_t
fatfs_space_alloc(fatfs_t *fatfs, size_t size)
{
    DEBUGF("trying to allocate %u/%u clusters", size, fatfs->free_count);
    if (fatfs->free_count < size)
        size = fatfs->free_count;
    if (size == 0) return -1;
    blkno_t clu = fatfs->free_first;
    blkno_t xclu = -1;
    blkno_t xxclu;
    xclu = clu;
    for (int i = 0; i < size; i++) {
        xxclu = xclu;
        xclu = fatfs_fatget(fatfs, xxclu);
        DEBUGF(" FAT12(%i) = %i", xxclu, xclu);
        if (xclu > fatfs->clu_used) break;
    }
    fatfs_fatset(fatfs, xxclu, 0xfff);
    fatfs->free_first = xclu;
    fatfs->free_count -= size;
    DEBUGF("allocated %u -> %u", clu, xclu);
    return clu;
}

void
fatfs_space_free(fatfs_t *fatfs, blkno_t blk)
{
    blkno_t clu;
    blkno_t nclu = blk;

    do {
        clu = nclu;
        nclu = fatfs_fatget(fatfs, clu);
        kprintf("%u<%u>[%u] ", clu, nclu, fatfs->clu_used);
        fatfs_fatset(fatfs, clu, fatfs->free_first);
        fatfs->free_first = clu;
        fatfs->free_count++;
    } while ( nclu <= fatfs->clu_used );
    kprintf("\n");
}

/*************************************************************************
 *
 */


/**
 * sprawdza pierwszy sektor nośnika
 * @param sblock odczytany pierwszy sektor
 * @return 0 gdy opis pasuje do FAT12, -1 w przeciłnym wypadku
 */
int
fatfs_check_sblock(const fatfs_sblock_t *sblock)
{
    // obecnie obsługujemy jedynie FAT12 na dyskietkach 1440kB
    if (sblock->media != 0xf0) {
        DEBUGF("media id (0x%x) is not 1440kB floppy (0xf0)",
            sblock->media);
        return -ENOTSUP;
    }
    return 0;
}

/**
 * tworzy opis FATFS i zbiera informacje z pierwszego sektora
 * @param sblock odczytany pierwszy sektor
 * @return utworzony deskryptor FATFS 
 */
fatfs_t *
fatfs_createfs(const fatfs_sblock_t *sblock)
{
    fatfs_t *fatfs = kmem_zalloc(sizeof(*fatfs), KM_SLEEP);
    fatfs->fats = sblock->tables;
    fatfs->fatbsize = FATFS_GET_TABLESIZE(sblock);
    fatfs->clubsize = sblock->clusize;
    fatfs->clusize = sblock->clusize * BSIZE;
    fatfs->maxroot = FATFS_GET_MAXROOT(sblock);
    fatfs->blkno_fat = FATFS_GET_RESERVED(sblock);
    fatfs->blkno_root = fatfs->blkno_fat + fatfs->fatbsize * fatfs->fats;
    fatfs->blkno_data = fatfs->blkno_root + (fatfs->maxroot*32)/BSIZE;
    fatfs->clu_free = FAT12_CLU_FREE;
    fatfs->clu_used = FAT12_CLU_USED;
    fatfs->clu_bad = FAT12_CLU_BAD;
    fatfs->clu_last = FAT12_CLU_LAST;
    fatfs->fat_get = fat12_get;
    fatfs->fat_set = fat12_set;
    fatfs->blkno_last = 2880-1;
    fatfs->fat = kmem_alloc(BSIZE*fatfs->fatbsize, KM_SLEEP);
    return fatfs;
}

/**
 * odczytuje pierwszą tablicę FAT z nośnika
 * @param fatfs deskryptor fatfs
 * @return 0 przy powodzeniu, lub bląd operacji wej-wyj
 */
int
fatfs_read_fat(fatfs_t *fatfs)
{
    iobuf_t *bp;
    bp = bio_read(fatfs->dev, fatfs->blkno_fat, fatfs->fatbsize*BSIZE);
    if ( ISSET(bp->flags, BIO_ERROR) ) {
        bio_release(bp);
        return -bp->errno;
    }
    mem_cpy(fatfs->fat, bp->addr, fatfs->fatbsize*BSIZE);
    bio_release(bp);
    return 0;
}

/**
 * odwzorowuje plik na sektor klastra na nośniku
 * @param node węzeł którego fragment chcemy odwzorować
 * @param off pozycja w pliku
 * @param size ograniczenie górne (w bajtach) na szukany ciąg klastrów
 *             (0 oznacza bez ograniczenia)
 * @param cont do uzupełnienia ilością ciągłych klastrów, NULL jeżeli ta
 *             informacja jest nieistotna.
 * @return zwraca numer sektora na nośniku zawierającego dany fragment pliku,
 *         lub -1 jeżeli taki sektor nie istnieje.
 *
 * Procedura zwraca numer sektora na nośniku, w którym rozpoczyna się klaster
 * przechowujący daną pozycję w pliku. Możliwe jest też zdobycie informacji
 * ile następnych klastrów występuje po sobie. W takim wypadku można je
 * wszystkie odczytać za pomocą jednego transferu danych z urządzenia,
 * co jest istotne dla wydajności systemu.
 */

blkno_t
fatfs_bmap(fatfs_node_t *node, off_t off, ssize_t s, size_t *cont)
{
#define has_content(clu) ((clu) <= fatfs->clu_used)
    fatfs_t *fatfs = node->fatfs;
    int clun = off/fatfs->clusize;
    int i;
    blkno_t clu = node->firstclu;
    if (cont) *cont = 0;
    for (i = 0; i != clun && has_content(clu); i++)
        clu = fatfs_fatget(fatfs, clu);
    if (i != clun || !has_content(clu)) {
        return -1;
    }
    if (cont) {
        int c = 1;
        ssize_t xs = 0;
        blkno_t nclu = fatfs->fat_get(fatfs->fat, clu);
        blkno_t pclu = clu;
        *cont = c;
        s -= off%fatfs->clusize;
        while ( has_content(nclu) && nclu == pclu+1 && (xs < s || s==0) ) {
            xs += fatfs->clusize;
            pclu = nclu;
            nclu = fatfs_fatget(fatfs, pclu);
            c++;
        }
         *cont = c * fatfs->clubsize;
    }
     return fatfs_cmap(fatfs, clu);
#undef has_content
}



blkno_t
fatfs_cmap(fatfs_t *fatfs, blkno_t clu)
{
    return fatfs->blkno_data + (clu-2)*fatfs->clubsize;
}

/*************************************************************************
 * Procedury obsługi tablic FAT
 */

blkno_t
fatfs_fatget(fatfs_t *fatfs, blkno_t i)
{
    return fatfs->fat_get(fatfs->fat, i);
}

void
fatfs_fatset(fatfs_t *fatfs, blkno_t i, blkno_t n)
{
    fatfs->fat_set(fatfs->fat, i, n);
}


blkno_t
fat12_get(const uint8_t *table, int i)
{
    int n = (3*i) / 2;
    uint16_t fat = *(uint16_t*)&(table[n]);
    if ( i & 1) {
        return (fat & 0xfff0) >>4;
    } else {
        return (fat & 0xfff);
    }
}

void
fat12_set(uint8_t *table, int i, blkno_t blk)
{
    int n = (3*i) / 2;
    uint16_t fat = *(uint16_t*)&(table[n]);
    uint16_t a,b;
    a = (fat & 0xfff);
    b = (fat & 0xfff0) >>4;
        if ( i & 1) {
            a &= 0x00f;
            b = blk;
        } else {
            a = blk;
            b &= 0xf00;
        }
    fat = (a&0xfff) | ((b << 4) & 0xfff0);
    *(uint16_t*)&(table[n]) = fat;
}


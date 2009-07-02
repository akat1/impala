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

/** @file 
 * G³ówny plik obs³ugi systemu plików MS FAT.
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
static vfs_sync_t      fatfs_sync;

static vfs_ops_t fatfs_ops = {
    fatfs_mount,
    fatfs_unmount,
    fatfs_getroot,
    fatfs_sync
};

int fatfs_check_sblock(const fatfs_sblock_t *fs);
fatfs_t *fatfs_createfs(const fatfs_sblock_t *fs);
int fatfs_read_fat(fatfs_t *fatfs);

blkno_t fat12_get(const uint8_t *, blkno_t);

/*************************************************************************
 * Procedury obs³ugi VFS.
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

/// pobiera opis g³ównego katalogu systemu plików
vnode_t *
fatfs_getroot(vfs_t *vfs)
{
    fatfs_t *fatfs = vfs->vfs_private;
    vref(fatfs->root);
    return fatfs->root;
}

/// synchronizuje system plików z no¶nikiem
void
fatfs_sync(vfs_t *vfs)
{
}

/*************************************************************************
 * Procedury obs³ugi VFS.
 */



/// zdobywa informacje o wolnym miejscu
void
fatfs_space_scan(fatfs_t *fatfs)
{
}

/// przydziela wolne miejsce na no¶niku
blkno_t
fatfs_space_alloc(size_t size)
{
    return -1;
}

/**
 * sprawdza pierwszy sektor no¶nika
 * @param sblock odczytany pierwszy sektor
 * @return 0 gdy opis pasuje do FAT12, -1 w przeci³nym wypadku
 */
int
fatfs_check_sblock(const fatfs_sblock_t *sblock)
{
    // obecnie obs³ugujemy jedynie FAT12 na dyskietkach 1440kB
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
    fatfs->fat = kmem_alloc(BSIZE*fatfs->fatbsize, KM_SLEEP);
    return fatfs;
}

/**
 * odczytuje pierwsz± tablicê FAT z no¶nika
 * @param fatfs deskryptor fatfs
 * @return 0 przy powodzeniu, lub bl±d operacji wej-wyj
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
 * odwzorowuje plik na sektor klastra na no¶niku
 * @param node wêze³ którego fragment chcemy odwzorowaæ
 * @param off pozycja w pliku
 * @param size ograniczenie górne (w bajtach) na szukany ci±g klastrów
 *             (0 oznacza bez ograniczenia)
 * @param cont do uzupe³nienia ilo¶ci± ci±g³ych klastrów, NULL je¿eli ta
 *             informacja jest nieistotna.
 * @return zwraca numer sektora na no¶niku zawieraj±cego dany fragment pliku,
 *         lub -1 je¿eli taki sektor nie istnieje.
 *
 * Procedura zwraca numer sektora na no¶niku, w którym rozpoczyna siê klaster
 * przechowuj±cy dan± pozycjê w pliku. Mo¿liwe jest te¿ zdobycie informacji
 * ile nastêpnych klastrów wystêpuje po sobie. W takim wypadku mo¿na je
 * wszystkie odczytaæ za pomoc± jednego transferu danych z urz±dzenia,
 * co jest istotne dla wydajno¶ci systemu.
 */

blkno_t
fatfs_bmap(fatfs_node_t *node, off_t off, ssize_t s, size_t *cont)
{
#define has_content(clu) ((clu) < fatfs->clu_bad)
    fatfs_t *fatfs = node->fatfs;
    int clun = off/fatfs->clusize;
    int i;
    blkno_t clu = node->firstclu;
    if (cont) *cont = 0;
    for (i = 0; i != clun && has_content(clu); i++)
        clu = fatfs->fat_get(fatfs->fat, clu);
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
            nclu = fatfs->fat_get(fatfs->fat, pclu);
            c++;
        }
         *cont = c;
    }
    return fatfs->blkno_data + (clu-2)*fatfs->clubsize;
#undef has_content
}


/*************************************************************************
 * Procedury obs³ugi tablic FAT
 */

blkno_t
fat12_get(const uint8_t *table, blkno_t i)
{
    int n = (3*i) / 2;
    uint16_t fat = *(uint16_t*)&(table[n]);
    uint16_t a,b;

    a = (fat & 0xfff);
    b = (fat & 0xfff0) >>4;
    if ( (3*i) & 1) {
        return b;
    } else {
        return a;
    }
}

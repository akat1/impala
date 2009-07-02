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

#ifndef __FS_FATFS_FATFS_H
#define __FS_FATFS_FATFS_H

struct fat16 {
    uint8_t     drive;
    uint8_t     reserved;
    uint8_t     extbootsig;
    uint8_t     serial[4];
    uint8_t     label[11];
    uint8_t     type[8];
    uint8_t     BOOT[448];
    uint8_t     bootsig[2];
};

struct fat32 {
    uint8_t     sectable[4];
    uint8_t     flags[2];
    uint8_t     version[2];
    uint8_t     cluroot[4];
    uint8_t     fsinfo_blkno[2];
    uint8_t     sblock_blkno[2];
    uint8_t     reserved[12];
    uint8_t     drive;
    uint8_t     reserved1;
    uint8_t     extbootsig;
    uint8_t     serial[4];
    uint8_t     label[11];
    uint8_t     type[8];
    uint8_t     BOOT[420];
    uint8_t     bootsig[2];
};

typedef struct fatfs_sblock fatfs_sblock_t;
struct fatfs_sblock {
    uint8_t             _jmp[3];
    uint8_t             oem[8];
    uint8_t             secsize[2];
    uint8_t             clusize;
    uint8_t             reserved[2];
    uint8_t             tables;
    uint8_t             maxroot[2];
    uint8_t             sectors[2];
    uint8_t             media;
    uint8_t             tablesize[2];
    uint8_t             sectrack[2];
    uint8_t             heads[2];
    uint8_t             hidden[4];
    uint8_t             sectors2[4];
    union {
            struct fat16    fat16;
            struct fat32    fat32;
    } un;
};

typedef struct fatfs_dentry fatfs_dentry_t;
struct fatfs_dentry {
    uint8_t         name[8];
    uint8_t         ext[3];
    uint8_t         attr;
    uint8_t         reserved;
    uint8_t         ctime_res;
    uint8_t         ctime[4];
    uint8_t         atime[2];
    uint8_t         ea_index[2];
    uint8_t         mtime[4];
    uint8_t         index[2];
    uint8_t         size[4];
};

typedef struct fatfs_lname fatfs_lname_t;
struct fatfs_lname {
    uint8_t         n;
    uint8_t         name1[10];
    uint8_t         attr;
    uint8_t         reserved;
    uint8_t         checksum;
    uint8_t         name2[12];
    uint8_t         clu[2];
    uint8_t         name3[4];
};


#define FATFS_GET_TABLESIZE(sblock) (*(uint16_t*)(&(sblock)->tablesize))
#define FATFS_GET_RESERVED(sblock) (*(uint16_t*)(&(sblock)->reserved))
#define FATFS_GET_MAXROOT(sblock) (*(uint16_t*)(&(sblock)->maxroot))
#define FATFS_D_GET_INDEX(dentry) (*(uint16_t*)(&(dentry)->index))
#define FATFS_D_GET_SIZE(dentry) (*(uint32_t*)(&(dentry)->size))

#ifdef __KERNEL

enum FATFS_NODE_TYPE {
    FATFS_ROOT,
    FATFS_DIR,
    FATFS_REG
};

enum FAT12_CLU {
    FAT12_CLU_FREE      = 0x000,
    FAT12_CLU_USED      = 0xfef,
    FAT12_CLU_BAD       = 0xff7,
    FAT12_CLU_LAST      = 0xff8
};

enum FATFS_ATTR {
    FATFS_RDONLY        = 1 << 0,
    FATFS_HIDDEN        = 1 << 1,
    FATFS_SYSTEM        = 1 << 2,
    FATFS_LABEL         = 1 << 3,
    FATFS_SUBDIR        = 1 << 4,
    FATFS_ARCHIVE       = 1 << 5,
    FATFS_DEVICE        = 1 << 6,
    FATFS_SKIP          = FATFS_HIDDEN|FATFS_LABEL,
    FATFS_LONGNAME      = 0x0f,
    FATFS_LONGNAME_LAST = 0x40,
    FATFS_LOGNNAME_DEL  = 0x80
};

typedef struct fatfs fatfs_t;
typedef struct fatfs_fat fatfs_fat_t;
typedef struct fatfs_node fatfs_node_t;
typedef struct fatfs_dir fatfs_dir_t;
typedef struct fatfs_dirent fatfs_dirent_t;
typedef blkno_t fatfs_get_t(const uint8_t *, blkno_t);
typedef void fatfs_set_t(uint8_t *, blkno_t, blkno_t);

/// opis systemu plików FAT
struct fatfs {
    vfs_t          *vfs;            ///< wirtualny system plików
    devd_t         *dev;            ///< urz±dzenie pod jakim jeste¶my
    int             type;           ///< typ
    int             fats;           ///< ilo¶æ tablic FAT
    int             fatbsize;       ///< rozmiar tablicy fat w sektorach
    int             clusize;        ///< rozmiar klastra w bajtach
    int             clubsize;       ///< rozmiar klastra w sektorach
    int             maxroot;        ///< maksymalna ilo¶æ pozycji w /
    vnode_t        *root;           ///< g³ówny katalog
    blkno_t         blkno_fat;      ///< po³o¿enie pierwszej tablicy FAT
    blkno_t         blkno_root;     ///< po³o¿enie g³ównego katalogu
    blkno_t         blkno_data;     ///< po³o¿enie danych
    uint8_t        *fat;            ///< wczytana tablica FAT

    uint            clu_free;       ///< idnetyfikator wolnego klastra
    uint            clu_used;       ///< identyfikator zajêtego klastra
    uint            clu_bad;        ///< identyfikator z³ego klastra
    int             clu_last;       ///< identyfikator ostatniego klastra

    fatfs_get_t  *fat_get;          ///< procedura odczytu z tablicy FAT
    fatfs_set_t  *fat_set;          ///< procedura zapisu do tablicy FAT
};

struct fatfs_node {
    char             name[256];
    int              type;
    int              attr;
    int              isroot;
    int              refcnt;
    size_t           size;
    blkno_t          firstclu;
    fatfs_dir_t     *dir;
    fatfs_dirent_t  *dirent;
    fatfs_t         *fatfs;
    vnode_t         *vnode;
};

struct fatfs_dir {
    list_t           dirents;
    fatfs_node_t    *node;
};

struct fatfs_dirent {
    char            name[256];
    int             attr;
    int             type;
    size_t          size;
    blkno_t         firstclu;
    fatfs_node_t   *node;
    list_node_t     L_dirents;
};

blkno_t fatfs_space_alloc(size_t size);
void fatfs_space_scan(fatfs_t *fatfs);
blkno_t fatfs_bmap(fatfs_node_t *, off_t, ssize_t, size_t *cont);
void fatfs_node_ref(fatfs_node_t *);
void fatfs_node_rel(fatfs_node_t *);
fatfs_node_t *fatfs_node_alloc(fatfs_t *, int type);
void fatfs_node_free(fatfs_node_t *);
int fatfs_node_getdir(fatfs_node_t *, fatfs_dir_t **);
vnode_t * fatfs_node_getv(fatfs_node_t *node);

int fatfs_dir_load(fatfs_node_t *);
void fatfs_dir_free(fatfs_dir_t *);
fatfs_node_t *fatfs_dir_lookup(fatfs_dir_t *, const char *name);
fatfs_node_t *fatfs_dir_create(fatfs_dir_t *, const char *name, int type);
void fatfs_dir_remove(fatfs_dir_t *, const char *path);
void fatfs_dir_release(fatfs_dir_t *, fatfs_node_t *n);
int fatfs_dir_getdents(fatfs_dir_t *, dirent_t *, int first, int n);
/// pomocnicze makro rzutuj±ce v_private
#define VTOFATFSN(v) ((fatfs_node_t*)(v)->v_private)
#define VTOFATFS(v) ((fatfs_t*)(v)->vfs_private)

#endif
#endif


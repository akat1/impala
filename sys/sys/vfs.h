/* Impala Operating System
 *
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

#ifndef __SYS_VFS_H
#define __SYS_VFS_H

/// informacje o punkcie podpięcia systemu plików
struct mountinfo {
    char    type[10];           ///< nazwa sterownika systemu plików
    char    mpoint[255];        ///< punkt zamontowania
    char    dev[20];            ///< urządzenie
};

#ifdef __KERNEL

#include <sys/vfs/vfs_types.h>
#include <sys/vfs/vfs_conf.h>
#include <sys/vfs/vfs_node.h>

/**
 * Struktura reprezentująca zamontowany system plików.
 */

struct vfs {
     vfs_ops_t    *vfs_ops;       ///< definicje operacji związanych z tym fs
     vnode_t      *vfs_mpoint;    ///< vnode który przykryliśmy tym fs
     devd_t       *vfs_mdev;      ///< urządzenie, używane przez fs
     void         *vfs_private;   ///< prywatne dane systemu plików
     vfs_conf_t   *vfs_conf;
     list_node_t   L_mountlist;   ///< węzeł z listy zamontowanych fs
};

int vfs_create(vfs_t **vsp, const char *type);
int vfs_mount(const char *name, vnode_t *mpoint, devd_t *dev);
int vfs_unmount(vnode_t *node);

int vfs_destroy(vfs_t *vp);
void vfs_init(void);
void vfs_mountroot(void);
int vfs_getinfos(off_t off, struct mountinfo *tab, size_t n);
extern vnode_t *rootvnode;

#define VFS_MOUNT(fs) (fs)->vfs_ops->vfs_mount((fs))
#define VFS_UNMOUNT(fs) (fs)->vfs_ops->vfs_unmount((fs))
#define VFS_SYNC(fs) (fs)->vfs_ops->vfs_sync((fs))
#define VFS_GETROOT(fs) (fs)->vfs_ops->vfs_getroot((fs))

typedef void vfs_init_t(void);
typedef int vfs_mount_t(vfs_t *fs);
typedef int vfs_unmount_t(vfs_t *fs);
typedef vnode_t * vfs_getroot_t(vfs_t *fs);
typedef void vfs_sync_t(vfs_t *fs);


/**
 * Abstrakcja na operacje na systemie plików
 */

struct vfs_ops {
    vfs_mount_t     *vfs_mount;
    vfs_unmount_t   *vfs_unmount;
    vfs_getroot_t   *vfs_getroot;
    vfs_sync_t      *vfs_sync;
};



#else

int getmountinfo(int off, struct mountinfo *tab, int n);


#endif
#endif

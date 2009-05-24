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

#ifndef __SYS_VFS_VFS_NODE_H
#define __SYS_VFS_VFS_NODE_H

#include <sys/vfs.h>

#ifdef __KERNEL

/**
 * Struktura reprezentuj±ca pojêcie pliku w sposób niezale¿ny od systemu plików
 */

struct vnode {
    int             v_type;        ///< typ vnode
    int             v_flags;       ///< flagi vnode
    int             v_refcnt;      ///< licznik referencji
    vfs_t          *v_vfs_mounted_here; ///< system plików tutaj zamontowany
    vfs_t          *v_vfs;         ///< system plików tego vnode
    vnode_ops_t    *v_ops;         ///< wska¼nik do vnode_ops z tego fs
    devd_t         *v_dev;         ///< urz±dzenie, je¶li to vnode urz±dzenia
    void           *v_private;     ///< prywatne dane fs-a... np. inode
};


enum {
    VNODE_TYPE_REG,
    VNODE_TYPE_DIR,
    VNODE_TYPE_DEV
};

enum {
    VNODE_FLAG_ROOT = 1<<0
};

/**
 *  Struktura ta jest po¶rednikiem przy pobieraniu b±d¼ zmianie atrybutów vnode
 */
struct vattr {
    int     va_mask;    ///< flagi wskazuj±ce które dane nas interesuj±
    int     va_uid;     ///< identyfikator w³a¶ciciela pliku
    int     va_gid;     ///< identyfikator grupy pliku
    int     va_mode;    ///< prawa dostêpu do pliku
    int     va_size;    ///< wielko¶æ pliku
    devd_t *va_dev;     ///< urz±dzenie reprezentowane przez ten plik
    int     va_type;    ///< typ vnode
};
typedef struct vattr vattr_t;

enum {
    VATTR_UID  = 1<<0,
    VATTR_GID  = 1<<1,
    VATTR_MODE = 1<<2,
    VATTR_DEV  = 1<<3,
    VATTR_TYPE = 1<<4,
    VATTR_SIZE = 1<<5
};

struct cpath {
    const char *path;
    const char *now;
};
typedef struct cpath cpath_t;


#define VOP_OPEN(v, fl, md) (v)->v_ops->vop_open((v), (fl), (md))
#define VOP_CREATE(v, name) (v)->v_ops->vop_create((v), (name))
#define VOP_CLOSE(v) (v)->v_ops->vop_close((v))
#define VOP_READ(v, uio) (v)->v_ops->vop_read((v), (uio))
#define VOP_WRITE(v, uio) (v)->v_ops->vop_write((v), (uio))
#define VOP_IOCTL(v, cmd, arg) (v)->v_ops->vop_ioctl((v), (cmd), (arg))
#define VOP_TRUNCATE(v, len) (v)->v_ops->vop_truncate((v), (len))
#define VOP_SEEK(v, off) (v)->v_ops->vop_seek((v), (off))
#define VOP_GETATTR(v, attr) (v)->v_ops->vop_getattr((v), (attr))
#define VOP_SETATTR(v, attr) (v)->v_ops->vop_setattr((v), (attr))
#define VOP_LOOKUP(v, w, p) (v)->v_ops->vop_lookup((v), (w), (p))

typedef int vnode_open_t(vnode_t *v, int flags, mode_t mode);
typedef int vnode_create_t(vnode_t *v, char *name);
typedef int vnode_close_t(vnode_t *v);
typedef int vnode_read_t(vnode_t *v, uio_t *u);
typedef int vnode_write_t(vnode_t *v, uio_t *u);
typedef int vnode_ioctl_t(vnode_t *v, int cmd, uintptr_t arg);
typedef int vnode_truncate_t(vnode_t *v, off_t len);
typedef int vnode_seek_t(vnode_t *v, off_t off);
typedef int vnode_getattr_t(vnode_t *v, vattr_t *attr);
typedef int vnode_setattr_t(vnode_t *v, vattr_t *attr);
typedef int vnode_lookup_t(vnode_t *v, vnode_t **vpp, cpath_t *path);
typedef int vnode_mkdir_t(vnode_t *v, char *path);


struct vnode_ops {
    vnode_open_t      *vop_open;
    vnode_create_t    *vop_create;
    vnode_close_t     *vop_close;
    vnode_read_t      *vop_read;
    vnode_write_t     *vop_write;
    vnode_ioctl_t     *vop_ioctl;
    vnode_seek_t      *vop_seek;
    vnode_truncate_t  *vop_truncate;
    vnode_getattr_t   *vop_getattr;
    vnode_setattr_t   *vop_setattr;
    vnode_lookup_t    *vop_lookup;
    vnode_mkdir_t     *vop_mkdir; 
/*    vnode_link_t      *vop_link;
    vnode_rmdir_t     *vop_rmdir; */
};


int lookupcp(vnode_t *sd, vnode_t **vpp, cpath_t *path, thread_t *thr);
int lookup(vnode_t *sd, vnode_t **vpp, const char *p, thread_t *thr);
int tmp_vnode_dev(devd_t *dev, vnode_t **vn);
int vnode_opendev(const char *devname, int mode, vnode_t **vn);
vnode_t* vnode_alloc(void);
void vrele(vnode_t *vn);
void vref(vnode_t *vn);


#endif
#endif


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
#include <fs/fatfs/fatfs.h>

#define NEW_INODE() kmem_zalloc(sizeof(fatfs_inode_t), KM_SLEEP)

static vnode_open_t      fatfs_open;
static vnode_create_t    fatfs_create;
static vnode_close_t     fatfs_close;
static vnode_read_t      fatfs_read;
static vnode_write_t     fatfs_write;
static vnode_ioctl_t     fatfs_ioctl;
static vnode_seek_t      fatfs_seek;
static vnode_truncate_t  fatfs_truncate;
static vnode_getattr_t   fatfs_getattr;
static vnode_setattr_t   fatfs_setattr;
static vnode_lookup_t    fatfs_lookup;
static vnode_mkdir_t     fatfs_mkdir;
static vnode_getdents_t  fatfs_getdents;
static vnode_readlink_t  fatfs_readlink;
static vnode_symlink_t   fatfs_symlink;
static vnode_access_t    fatfs_access;
static vnode_sync_t      fatfs_sync;
static vnode_inactive_t  fatfs_inactive;


static vnode_ops_t fatfs_node_ops = {
    fatfs_open,
    fatfs_create,
    fatfs_close,
    fatfs_read,
    fatfs_write,
    fatfs_ioctl,
    fatfs_seek,
    fatfs_truncate,
    fatfs_getattr,
    fatfs_setattr,
    fatfs_lookup,
    fatfs_mkdir,
    fatfs_getdents,
    fatfs_readlink,
    fatfs_symlink,
    fatfs_access,
    fatfs_sync,
    fatfs_inactive
};

vnode_t *
fatfs_getvnode(fatfs_inode_t *inode)
{
    if (inode->vn == NULL) {
        inode->vn = vnode_alloc();
        inode->vn->v_ops = &fatfs_node_ops;
        inode->vn->v_private = inode;
        inode->vn->v_vfs = inode->fatfs->vfs;
        if (inode->type == FATFS_REG) {
            inode->vn->v_type = VNODE_TYPE_REG;
        } else
        if (inode->type == FATFS_DIR) {
            inode->vn->v_type = VNODE_TYPE_DIR;
        } else
        if (inode->type == FATFS_ROOT) {
            inode->vn->v_type = VNODE_TYPE_DIR;
            inode->vn->v_flags = VNODE_FLAG_ROOT;
        }
    } else vref(inode->vn);
    return inode->vn;
}

void
fatfs_inode_prepare(fatfs_t *fatfs, fatfs_inode_t *inode, int type)
{
    inode->type = type;
    inode->fatfs = fatfs;
    inode->clubuf = fatfs_clu_alloc(fatfs);
    inode->clunum = -1;
    switch (type) {
        case FATFS_REG:
            break;
        case FATFS_DIR:
        case FATFS_ROOT:
            LIST_CREATE(&inode->un.dir.dirents, fatfs_dirent_t, L_dirents,
                FALSE);
            break;
    }
}

void
fatfs_inode_create(fatfs_t *fatfs, fatfs_dirent_t *dirent)
{
    KASSERT( dirent->inode == NULL );
    dirent->inode = NEW_INODE();
    str_cpy(dirent->inode->name, dirent->name);
    fatfs_inode_prepare(fatfs, dirent->inode, 0);
    dirent->inode->clustart = dirent->clustart;
    dirent->inode->size = dirent->size;
    if (dirent->attr & FATFS_ARCHIVE) {
        dirent->inode->type = FATFS_REG;
    } else
    if (dirent->attr & FATFS_SUBDIR) {
        dirent->inode->type = FATFS_DIR;
    } else panic("unknown type dirent %b", dirent->inode->type);
    fatfs_getvnode(dirent->inode);
    vref(dirent->inode->vn);
}


/*============================================================================
 * Operacje na v-wê¼le
 */

int
fatfs_open(vnode_t *v, int flags, mode_t mode)
{
    return 0;
}

int
fatfs_create(vnode_t *v, vnode_t **vpp, const char *name,
    vattr_t *attr)
{
    DEBUGF("create not supported");
    return -ENOTSUP;
}

int
fatfs_close(vnode_t *v)
{
    return 0;
}

int
fatfs_read(vnode_t *v, uio_t *u, int flags)
{
    fatfs_inode_t *inode = v->v_private;
    fatfs_t *fatfs = inode->fatfs;
    if (u->offset >= inode->size) return 0;
    int clu = inode->clustart;
    off_t window = 0;
    size_t wsize = fatfs->clusize*fatfs->secsize;
    size_t xfer = 0;
    u->resid = u->size;
    do {
        if ( INRNG_COL(u->offset,window,wsize) ) {
            off_t off = u->offset - window;
            int min = MIN(wsize-off, u->resid);
            if (fatfs_clu_read(fatfs, clu, inode)) return -EIO;
            char *buf = inode->clubuf;
            uio_move(buf+off, min , u);
            xfer += min;
        }
        window += wsize;
    } while ( FATFS_UNTIL_EOF(fatfs, clu) && u->resid );
    return xfer;
}

int
fatfs_write(vnode_t *v, uio_t *u, int flags)
{
    DEBUGF("write not supported");
    return -ENOTSUP;
}

int
fatfs_ioctl(vnode_t *v, int cmd, uintptr_t arg)
{
    DEBUGF("ioctl not supported");
    return -ENOTSUP;
}

int
fatfs_truncate(vnode_t *v, off_t len)
{
    DEBUGF("truncate not supported");
    return -ENOTSUP;
}

int
fatfs_seek(vnode_t *v, off_t off)
{
    DEBUGF("seek not supported");
    return -ENOTSUP;
}

int
fatfs_getattr(vnode_t *v, vattr_t *attr)
{
    fatfs_inode_t *inode = v->v_private;
    attr->va_uid = attr->va_gid = 0;
    attr->va_mode = S_IRWXU|S_IRWXG|S_IRWXO;
    attr->va_type = v->v_type;
    attr->va_blksize = inode->fatfs->secsize;
    attr->va_blocks = -1;
    attr->va_nlink = 0;
    attr->va_size = inode->size;
    return 0;
}

int
fatfs_setattr(vnode_t *v, vattr_t *attr)
{
    DEBUGF("setattr not supported");
    return -ENOTSUP;
}

int
fatfs_lookup(vnode_t *v, vnode_t **vpp, lkp_state_t *state)
{
    if (v->v_type != VNODE_TYPE_DIR) return -EINVAL;
    char name[256];
    int i;
    for (i = 0; i < 255 && state->now[i] != 0 && state->now[i] != '/' ; i++) {
        name[i] = state->now[i];
    }
    name[i] = 0;
    fatfs_inode_t *ninode;
    if (fatfs_dirent_lookup(v->v_private, &ninode, name)) {
        return -ENOENT;
    }
    ninode->vn = fatfs_getvnode(ninode);
    *vpp = ninode->vn;
    state->now+=str_len(ninode->name);
    return 0;
}

int
fatfs_mkdir(vnode_t *v, vnode_t **vpp, const char *path,
                           vattr_t *attr)
{
    DEBUGF("mkdir not supported");
    return -ENOTSUP;
}

int
fatfs_getdents(vnode_t *v, dirent_t *dents, int first, int count)
{
    DEBUGF("getdents not supported");
    return -ENOTSUP;
}

int
fatfs_readlink(vnode_t *v, char *buf, int bsize)
{
    return -ENOTSUP;
}

int
fatfs_symlink(vnode_t *v, char *name, char *dst)
{
    return -ENOTSUP;
}

int
fatfs_access(vnode_t *v, int mode, pcred_t *c)
{
    return 0;
}

int
fatfs_sync(vnode_t *v)
{
    DEBUGF("fsync not supported");
    return -ENOTSUP;
}

int
fatfs_inactive(vnode_t *v)
{
    fatfs_inode_t *inode = v->v_private;
    DEBUGF("inactive (%s)", inode->name);
    return 0;
}


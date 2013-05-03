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
#include <sys/vfs/vfs_gen.h>
#include <fs/fatfs/fatfs.h>


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
    fatfs_inactive,
    vfs_gen_lock,
    vfs_gen_unlock
};

/*============================================================================
 * Operacje na v-węźle
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
    fatfs_node_t *node = v->v_private;
    fatfs_node_t *newnode;
    fatfs_dir_t *dir;
    int e;

    e = fatfs_node_getdir(node, &dir);
    if (e) {
        return e;
    }
    int t = (attr->va_type == VNODE_TYPE_DIR)? FATFS_DIR : FATFS_REG;
    newnode = fatfs_dir_create(dir, name, t, 0);
    if (!newnode) {
        return -EINVAL;
    }
    *vpp = fatfs_node_getv(newnode);
    VFS_SYNC(node->fatfs->vfs);
    return 0;
}

int
fatfs_close(vnode_t *v)
{
    return 0;
}

int
fatfs_read(vnode_t *v, uio_t *u, int flags)
{
    ssize_t r;
    fatfs_node_t *node = VTOFATFSN(v);
    r = fatfs_node_read(node, u, flags);
    return r;
}

int
fatfs_write(vnode_t *v, uio_t *u, int flags)
{
    ssize_t r;
    fatfs_node_t *node = VTOFATFSN(v);
    if (node->size < u->offset + u->size) {
        r = fatfs_node_truncate(node, u->offset + u->size);
        if (r) return r;
        VFS_SYNC(node->fatfs->vfs);
    }
    r = fatfs_node_write(node, u, flags);
    return r;
}

int
fatfs_ioctl(vnode_t *v, int cmd, uintptr_t arg)
{
    return -EINVAL;
}

int
fatfs_truncate(vnode_t *v, off_t len)
{
    int err = 0;
    fatfs_node_t *node = v->v_private;
    size_t oldsize = node->size;
    if (node->type != FATFS_REG || v->v_type != VNODE_TYPE_REG)
        return -EISDIR;
    fatfs_node_truncate(node, len);
    if (node->dirent) {
        fatfs_dir_sync(node->dirent->dirnode->dir);
    }
    if (oldsize < node->size) {
        uio_t u;
        iovec_t iov;
        u.iovs = &iov;
        u.iovcnt = 1;
        u.space = UIO_SYSSPACE;
        u.size = node->size - oldsize;
        u.resid = u.size;
        u.completed = 0;
        iov.iov_len = u.resid;
        iov.iov_base = kmem_zalloc(u.resid, KM_SLEEP);
        err = fatfs_node_write(node, &u, 0);
        err = (err < 0)? err : 0;
    }
    VFS_SYNC(node->fatfs->vfs);
    return err;
}

int
fatfs_seek(vnode_t *v, off_t off)
{
    return -ENOTSUP;
}

int
fatfs_getattr(vnode_t *v, vattr_t *attr)
{
    fatfs_node_t *node = VTOFATFSN(v);
    fatfs_t *fatfs = node->fatfs;
    if ( ISSET(attr->va_mask, VATTR_UID) )
        attr->va_uid = 0;
    if ( ISSET(attr->va_mask, VATTR_GID) )
        attr->va_gid = 0;
    if ( ISSET(attr->va_mask, VATTR_MODE) )
        attr->va_mode = 0777; 
    if ( ISSET(attr->va_mask, VATTR_SIZE) )
        attr->va_size = node->size;
    if ( ISSET(attr->va_mask, VATTR_TYPE) )
        attr->va_type = v->v_type;
    if ( ISSET(attr->va_mask, VATTR_NLINK) )
        attr->va_nlink = 1;
    if ( ISSET(attr->va_mask, VATTR_INO) ) {
        attr->va_ino = node->firstclu;
    }
    if ( ISSET(attr->va_mask, VATTR_BLK) ) {
        attr->va_blksize = fatfs->clusize;
        attr->va_blocks = (node->size+fatfs->clusize-1)/fatfs->clusize;
    }
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
    char bufname[256];
    int i;
    int err;
    fatfs_node_t *rnode = 0;
    fatfs_dir_t *dir;

    if (v->v_type != VNODE_TYPE_DIR) return -ENOTDIR;
    for (i = 0; i < 256 && state->now[i] && state->now[i] != '/'; i++);
    str_ncpy(bufname, state->now, i);
    bufname[i] = 0;
    err = fatfs_node_getdir(VTOFATFSN(v), &dir);
    if (err) return err;
    KASSERT(dir != NULL);
    rnode = fatfs_dir_lookup(dir, bufname);
    if (rnode == NULL) return -ENOENT;
    state->now += i;
    *vpp = fatfs_node_getv(rnode);
    return 0;
}

int
fatfs_mkdir(vnode_t *v, vnode_t **vpp, const char *path,
                           vattr_t *attr)
{
    int err;
    blkno_t clu;
    fatfs_node_t *node = v->v_private;
    fatfs_t *fatfs = node->fatfs;
    fatfs_node_t *newnode;
    fatfs_dir_t *dir;
    iobuf_t *bp;

    if ((node->type != FATFS_DIR && node->type != FATFS_ROOT) ||
            v->v_type != VNODE_TYPE_DIR)
        return -ENOTDIR;

    err = fatfs_node_getdir(node, &dir);
    if (err) return err;

    clu = fatfs_space_alloc(fatfs, 1);
    if (clu == -1) {
        return -ENOSPC;
    }

    bp = bio_getblk(fatfs->dev, fatfs_cmap(fatfs,clu), fatfs->clusize);
    mem_zero(bp->addr, bp->size);
    bio_write(bp);
    bio_wait(bp);
    if ( ISSET(bp->flags, BIO_ERROR) ) {
        err = -bp->errno;
        bio_release(bp);
        fatfs_space_free(fatfs, clu);
        return err;
    }
    bio_release(bp);
    newnode = fatfs_dir_create(dir, path, FATFS_DIR, clu);

    *vpp = fatfs_node_getv(newnode);
    VFS_SYNC(fatfs->vfs);
    return 0;
}

int
fatfs_getdents(vnode_t *v, dirent_t *dents, int first, int count)
{
    int err;
    fatfs_node_t *node = VTOFATFSN(v);
    fatfs_dir_t *dir;
    if (node->type != FATFS_ROOT && node->type != FATFS_DIR)
        return -ENOTDIR;

    err = fatfs_node_getdir(node, &dir);
    if (err) return err;
    KASSERT(dir != NULL);
    count /= sizeof(dirent_t);
    return fatfs_dir_getdents(node->dir, dents, first, count);
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
#if 0
    DEBUGF("fsync not supported");
    return -ENOTSUP;
#endif
    return -EOK;
}

int
fatfs_inactive(vnode_t *v)
{
    fatfs_node_t *fatfs = VTOFATFSN(v);
    fatfs->vnode = 0;
#if 0 // węzły zostawiamy w katalogach
    if (fatfs->dirent) {
        // jeżeli w pamięci jest katalog
        fatfs->dirent->node = 0;
        fatfs->dirent = 0;
        fatfs_node_rel(fatfs);
    }
#endif
    fatfs_node_rel(fatfs);
    return 0;
}


/*============================================================================
 * 
 */


int
fatfs_node_read(fatfs_node_t *node, uio_t *u, int flags)
{
    iobuf_t *bp;
    blkno_t blk;
    off_t off = u->offset % node->fatfs->clusize;
    size_t cont = 1;
    u->resid = MIN(u->resid, MAX(0, node->size - u->offset));
    u->completed = 0;
    while ((blk=fatfs_bmap(node,u->offset,u->resid,&cont)) != -1 && u->resid) {
        size_t size = cont*node->fatfs->clusize;
        bp = bio_read(node->fatfs->dev, blk, cont*node->fatfs->clusize);
        if ( ISSET(bp->flags, BIO_ERROR) ) {
            return -EIO;
            bio_release(bp);
        }
        size -= off;
        uio_move(bp->addr + off, MIN(size, u->resid), u);
        off = 0;
        bio_release(bp);
    }
    return u->completed;
}

int
fatfs_node_write(fatfs_node_t *node, uio_t *u, int flags)
{
    iobuf_t *bp;
    blkno_t blk;
    off_t off = u->offset % node->fatfs->clusize;
    size_t cont = 1;
    u->resid = MIN(u->resid, MAX(0, node->size - u->offset));
    u->completed = 0;
    while ((blk=fatfs_bmap(node,u->offset,u->resid,&cont)) != -1 && u->resid) {
        size_t size = cont*node->fatfs->clusize;
        bp = bio_getblk(node->fatfs->dev, blk, cont*node->fatfs->clusize);
        size -= off;
        uio_move(bp->addr + off, MIN(size, u->resid), u);
        off = 0;
        bio_write(bp);
        bio_wait(bp);
        if ( ISSET(bp->flags, BIO_ERROR) ) {
            int err = bp->errno;
            bio_release(bp);
            return -err;
        }
        bio_release(bp);
    }
    return u->completed;
}


vnode_t *
fatfs_node_getv(fatfs_node_t *node)
{
    if (node->vnode) {
        vref(node->vnode);
        return node->vnode;
    }
    node->vnode = vnode_alloc();
    node->vnode->v_vfs = node->fatfs->vfs;
    node->vnode->v_ops = &fatfs_node_ops;
    node->vnode->v_private = node;
    switch (node->type) {
        case FATFS_ROOT:
            node->vnode->v_flags |= VNODE_FLAG_ROOT;
        case FATFS_DIR:
            node->vnode->v_type = VNODE_TYPE_DIR;
            break;
        case FATFS_REG:
            node->vnode->v_type = VNODE_TYPE_REG;
            break;
    }
    return node->vnode;
}

void
fatfs_node_ref(fatfs_node_t *node)
{
    KASSERT(node->refcnt > 0);
    node->refcnt++;
}

void
fatfs_node_rel(fatfs_node_t *node)
{
    KASSERT(node->refcnt > 0);
    node->refcnt--;
    if (node->refcnt == 0) {
        fatfs_node_free(node);
    }
}

void
fatfs_dirent_sync(fatfs_node_t *node)
{
    if (!node->dirent) return;
    node->dirent->size = node->size;
    node->dirent->firstclu = node->firstclu;

}

fatfs_node_t *
fatfs_node_alloc(fatfs_t *fatfs, int type)
{
    fatfs_node_t *node = kmem_zalloc(sizeof(*node), KM_SLEEP);
    node->fatfs = fatfs;
    node->refcnt = 1;
    node->type = type;
    return node;
}

void
fatfs_node_free(fatfs_node_t *node)
{
    KASSERT(node->refcnt == 0);
    KASSERT(node->dirent == NULL);
    if ((node->type == FATFS_DIR || node->type == FATFS_ROOT) && node->dir) {
        fatfs_dir_free(node->dir);
    }
    kmem_free(node);
}

int
fatfs_node_getdir(fatfs_node_t *node, fatfs_dir_t **r)
{
    int err;
    if (node->dir == NULL) {
        err = fatfs_dir_load(node);
        if (err) {
            return err;
        }
    }
    *r = node->dir;
    return 0;
}

int
fatfs_node_truncate(fatfs_node_t *node, off_t off)
{
    fatfs_t *fatfs = node->fatfs;
    size_t need = (off+fatfs->clusize-1) / fatfs->clusize;
    size_t have = node->csize;
    blkno_t clu = node->firstclu;
    blkno_t xclu = node->firstclu;
    node->size = off;
    node->csize = need;

    if (need == have) {
        goto end;
    } else
    if (need == 0) {
        if (node->firstclu) {
            fatfs_space_free(fatfs, node->firstclu);
            node->firstclu = 0;
        }
        goto end;
    } else
    if (have == 0) {
        node->firstclu = fatfs_space_alloc(fatfs, need);
        goto end;
    }

    // jedziemy na koniec mniejszego łańcucha
    for (int i = 0; i < need && i < have; i++) {
        xclu = clu;
        clu = fatfs_fatget(fatfs, clu);
    }
    if (need < have) {
        fatfs_space_free(fatfs, clu);
        fatfs_fatset(fatfs, xclu, fatfs->clu_last);
    } else {
        blkno_t blk = fatfs_space_alloc(fatfs, need-have);
        fatfs_fatset(fatfs, xclu, blk);
    }
end:
    fatfs_dirent_sync(node);
    if (node->dirent && node->dirent->dirnode)
        fatfs_dir_sync(node->dirent->dirnode->dir);
    return 0;
}


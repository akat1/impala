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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/bio.h>
#include <sys/vfs.h>
#include <fs/mfs/mfs.h>
#include <sys/kmem.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/uio.h>
#include <sys/kmem.h>
#include <fs/mfs/mfs_internal.h>

vfs_init_t           mfs_init;
static vfs_mount_t   mfs_mount;
static vfs_unmount_t mfs_unmount;
static vfs_sync_t    mfs_sync;
static vfs_getroot_t mfs_getroot;

static vfs_ops_t mfs_ops = {
    .vfs_mount = mfs_mount,
    .vfs_unmount = mfs_unmount,
    .vfs_getroot = mfs_getroot,
    .vfs_sync = mfs_sync
};


mfs_node_t*
_alloc_node()
{
    mfs_node_t *n = kmem_zalloc(sizeof(mfs_node_t), KM_SLEEP);
    n->vnode = NULL;
    return n;
}


int
_get_vnode(mfs_node_t *en, vnode_t **vpp, vfs_t *fs)
{
    if(en->vnode == NULL) {
        vnode_t *res = vnode_alloc();
        if(!res)
            return -ENOMEM;
        res->v_vfs = fs;
        res->v_flags = (en->parent)?0:VNODE_FLAG_ROOT;
        res->v_ops = &mfs_vnode_ops;
        res->v_type = (en->type==MFS_TYPE_DIR)?
                        VNODE_TYPE_DIR:VNODE_TYPE_REG;
        res->v_private = en;
        en->vnode = res;
    } else vref(en->vnode);
    *vpp = en->vnode;
    return 0;
}

/*============================================================================
 * obs�uga VFS
 */

void
mfs_init()
{
    vfs_register("mfs", &mfs_ops);
}

int
mfs_mount(vfs_t *fs)
{
    vnode_t *dv = NULL;
    if(fs->vfs_mdev) {
        tmp_vnode_dev(fs->vfs_mdev, &dv);
        if(!dv) {
            DEBUGF("cannot open dev");
            return -1;
        }
    }
    mfs_data_t *mfs = kmem_zalloc(sizeof(mfs_data_t), KM_SLEEP);
    mfs->rootvnode = NULL;
    mfs->rootinode = NULL;
    mfs->cache_blk = kmem_cache_create("mfsblk", sizeof(mfs_blk_t),
        NULL, NULL);
    fs->vfs_private = mfs;
    mfs->rootinode = _alloc_node();
    mfs->rootinode->mfs = mfs;
    mfs->rootinode->name = "";
    mfs->rootinode->size = 0;
    mfs->rootinode->type = MFS_TYPE_DIR;

    return 0;
}

int
mfs_unmount(vfs_t *fs)
{
    return 0;
}

void
mfs_sync(vfs_t *fs)
{
}

vnode_t *
mfs_getroot(vfs_t *fs)
{
    mfs_data_t *mfs = fs->vfs_private;
    if(mfs->rootvnode == NULL) {
        vnode_t *rn = vnode_alloc();
        rn->v_vfs = fs;
        rn->v_flags = VNODE_FLAG_ROOT;
        rn->v_ops = &mfs_vnode_ops;
        rn->v_type = VNODE_TYPE_DIR;
        rn->v_private = mfs->rootinode;
        mfs->rootvnode = rn;
    } else vref(mfs->rootvnode);
    return mfs->rootvnode;
}


/*============================================================================
 * obs�uga bloczk�w
 */

int
mfs_blk_write(mfs_node_t *n, uio_t *uio)
{
    if (uio->size == 0) return 0;
    if (n->size < uio->offset+uio->size) {
        if (mfs_blk_set_area(n, uio->offset+uio->size))
            return -ENOMEM;
    }
    mfs_blk_t *bs = list_get_n(&n->blks, uio->offset/512);
    int off = uio->offset % 512;
    while (uio->resid) {
        int min = MIN(uio->resid, 512-off);
        if (uio_move(bs->data+off, min, uio) == -1)
            return -1;
        bs = list_next(&n->blks, bs);
        off = 0;
    }
    return uio->size;
}

int
mfs_blk_read(mfs_node_t *n, uio_t *uio)
{
    size_t res = 0;
    if (uio->size == 0) return 0;
    mfs_blk_t *bs = list_get_n(&n->blks, uio->offset/512);
    int off = uio->offset % 512;
    while (uio->resid && bs) {
        int min = MIN(uio->resid, 512-off);
        if (uio_move(bs->data+off, min, uio) == -1) return -1;
        res += min;
        bs = list_next(&n->blks, bs);
        off = 0;
    }
    return res;
}

int
mfs_blk_set_area(mfs_node_t *n, size_t s)
{
    mfs_data_t *mfs = n->mfs;
    int using = (s+511) / 512;
    int last = s%512;
    // skracamy list�
    while (list_length(&n->blks) > using) {
        mfs_blk_t *blk = list_extract_first(&n->blks);
        kmem_cache_free(mfs->cache_blk, blk);
    }
    while (list_length(&n->blks) < using) {
        mfs_blk_t *blk = kmem_cache_alloc(mfs->cache_blk, KM_SLEEP);
        if (blk == NULL) return -1;
        mem_zero(blk, sizeof(*blk));
        list_insert_tail(&n->blks, blk);
    }

    if (last) {
        // je�eli skracali�my plik to trzeba wyzerowa� bajty w bloczku
        // wykraczaj�ce za niego.
        mfs_blk_t *blk = list_tail(&n->blks);
        mem_zero(blk->data+last, 512-last);
    }
    n->size = s;
    return 0;
}

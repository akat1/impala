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
#include <sys/vfs/vfs_gen.h>
#include <fs/mfs/mfs.h>
#include <sys/kmem.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/uio.h>
#include <sys/kmem.h>
#include <sys/proc.h>
#include <fs/mfs/mfs_internal.h>


static vnode_open_t mfs_open;
static vnode_create_t mfs_create;
static vnode_close_t mfs_close;
static vnode_read_t mfs_read;
static vnode_write_t mfs_write;
static vnode_ioctl_t mfs_ioctl;
static vnode_seek_t mfs_seek;
static vnode_truncate_t mfs_truncate;
static vnode_getattr_t mfs_getattr;
static vnode_setattr_t mfs_setattr;
static vnode_lookup_t mfs_lookup;
static vnode_mkdir_t mfs_mkdir;
static vnode_getdents_t mfs_getdents;
static vnode_readlink_t mfs_readlink;
static vnode_symlink_t mfs_symlink;
static vnode_access_t mfs_access;
static vnode_inactive_t mfs_inactive;
static int pc_cmp(lkp_state_t *path, const char *fname);


vnode_ops_t mfs_vnode_ops = {
    .vop_open = mfs_open,
    .vop_create = mfs_create,
    .vop_close = mfs_close,
    .vop_read = mfs_read,
    .vop_write = mfs_write,
    .vop_ioctl = mfs_ioctl,
    .vop_seek = mfs_seek,
    .vop_truncate = mfs_truncate,
    .vop_getattr = mfs_getattr,
    .vop_setattr = mfs_setattr,
    .vop_lookup = mfs_lookup,
    .vop_mkdir = mfs_mkdir,
    .vop_getdents = mfs_getdents,
    .vop_readlink = mfs_readlink,
    .vop_access = mfs_access,
    .vop_symlink = mfs_symlink,
    .vop_inactive = mfs_inactive,
    .vop_lock = vfs_gen_lock,
    .vop_unlock = vfs_gen_unlock
};

int
mfs_nodecreate(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    char namex[256];
    str_ncpy(namex, name, 256);
    name = namex;
    // szybki hack
    if (namex[str_len(name)-1] == '/')
        namex[str_len(name)-1] = 0;

    *vpp = NULL;
    if(!name || !attr)
        return -EINVAL;
    if(vn->v_type != VNODE_TYPE_DIR)
        return -ENOTDIR;
    mfs_node_t *pnode = vn->v_private;
    mfs_node_t *node = _alloc_node();
    if(!node)
        return -ENOMEM;
    node->name = str_dup(name);
    node->size = attr->va_size;
    node->mfs = pnode->mfs;
    node->alloc_size = 0;
    node->type = VNODE_TO_MFS_TYPE(attr->va_type);
    node->attr = attr->va_mode;
    node->data = (attr->va_size>0)?kmem_alloc(attr->va_size, KM_SLEEP):NULL;
    if(node->data == NULL)
        node->size = 0;
    node->parent = pnode;
    node->child = NULL;
    node->next = pnode->child;
    pnode->child = node;
    LIST_CREATE(&node->blks, mfs_blk_t, L_blks, FALSE);
    return _get_vnode(node, vpp, vn->v_vfs);
}


/*============================================================================
 * obs³uga v-wêz³a
 */

int
mfs_open(vnode_t *vn, int flags, mode_t mode)
{
    return 0;
}


int
mfs_create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    return mfs_nodecreate(vn, vpp, name, attr);
}

int
mfs_mkdir(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    return mfs_nodecreate(vn, vpp, name, attr);
}

int
mfs_close(vnode_t *vn)
{
    return 0;
}

int
mfs_read(vnode_t *vn, uio_t *u, int flags)
{
    if(vn->v_type != VNODE_TYPE_REG)
        return -EINVAL;
    return mfs_blk_read(vn->v_private, u);
}

int
mfs_write(vnode_t *vn, uio_t *u, int flags)
{
    if(vn->v_type != VNODE_TYPE_REG)
        return -EINVAL;
    return mfs_blk_write(vn->v_private, u);  
}

int
mfs_ioctl(vnode_t *vn, int cmd, uintptr_t arg)
{
    return -ENOTSUP;
}

int
mfs_seek(vnode_t *vn, off_t off)
{
    mfs_node_t *n = vn->v_private;
    if(n->size > off)
        return 0;
    return mfs_blk_set_area(n, off);
}

int
mfs_truncate(vnode_t *vn, off_t off)
{
    mfs_node_t *n = vn->v_private;
    if(off == n->size)
        return 0;
    if(off > 10000000)
        return -EFBIG; //póki co MFS nie chce du¿ych plików ;)
    return mfs_blk_set_area(n, off);
}

int
mfs_getattr(vnode_t *vn, vattr_t *attr)
{
    mfs_node_t *node = vn->v_private;
    if(!node)
        return -EINVAL;
    if(attr->va_mask & VATTR_SIZE)
        attr->va_size = node->size;
    if(attr->va_mask & VATTR_TYPE)
        attr->va_type = MFS_TO_VNODE_TYPE(node->type);
    if(attr->va_mask & VATTR_MODE)
        attr->va_mode = node->attr;
    if(attr->va_mask & VATTR_UID)
        attr->va_uid = node->uid;
    if(attr->va_mask & VATTR_GID)
        attr->va_gid = node->gid;
    if(attr->va_mask & VATTR_DEV)
        attr->va_dev = NULL;
    if(attr->va_mask & VATTR_NLINK)
        attr->va_nlink = 1; //mfs nie wspiera hardlinków
    if (attr->va_mask & VATTR_INO)
        attr->va_ino = (ino_t) node;
    return 0;
}

int
mfs_setattr(vnode_t *vn, vattr_t *attr)
{
    mfs_node_t *node = vn->v_private;
    if(attr->va_mask & (VATTR_DEV))
        return -EINVAL;
    if(attr->va_mask & VATTR_UID)
        node->uid = attr->va_uid;
    if(attr->va_mask & VATTR_GID)
        node->gid = attr->va_gid;
    if(attr->va_mask & VATTR_SIZE) {
        if(attr->va_size > node->size)
            return -EINVAL;
        else
            node->size = attr->va_size; //obicinamy
    }
    if(attr->va_mask & VATTR_TYPE) {
        node->type = VNODE_TO_MFS_TYPE(attr->va_type);
    }
    if(attr->va_mask & VATTR_MODE)
        node->attr = attr->va_mode;
    return 0;
}

int
mfs_readlink(vnode_t *v, char *buf, int bsize)
{
    mfs_node_t *n = v->v_private;
    if(n->type != MFS_TYPE_LNK)
        return -EINVAL;
    int len = MIN(bsize, n->size);
    mem_cpy(buf, n->data, len);
    return len;
}

int
mfs_symlink(vnode_t *v, char *name, char *dst)
{
    vattr_t va;
    va.va_dev = NULL;
    va.va_type = VNODE_TYPE_LNK;
    va.va_uid = 0;// thread w arg?
    va.va_gid = 0;
    va.va_size = str_len(dst);
    va.va_mode = 0777;
    vnode_t *vn;
    int err = mfs_nodecreate(v, &vn, name, &va);
    if(err)
        return err;
    mfs_node_t *n = vn->v_private;
    mem_cpy(n->data, dst, n->size);
    vrele(vn);
    return 0;
}

int
mfs_access(vnode_t *vn, int mode, pcred_t *cred)
{
    if(!cred)
        return 0;
    mfs_node_t *n = vn->v_private;
    return vnode_access_ok(n->uid, n->gid, n->attr&0777, mode, cred);
}

int
mfs_inactive(vnode_t *vn)
{
    mfs_node_t *n = vn->v_private;
    n->vnode = NULL;
    return 0;
}

int
pc_cmp(lkp_state_t *path, const char *fname)
{
    const char *c = path->now;
    while(*c && *c!='/' && *fname)
        if(*(c++)!=*(fname++))
            return 1;
    if(*fname || (*c && *c!='/'))
        return 1;
    return 0;
}

int
mfs_lookup(vnode_t *vn, vnode_t **vpp, lkp_state_t *path)
{
    *vpp = NULL;
    mfs_node_t *en = vn->v_private;
    if(!pc_cmp(path, "..")) {
        path->now+=2;
        if(!en->parent) {
            *vpp = vn;
            vref(vn);
            return 0;
        }
        en = en->parent;
        _get_vnode(en, vpp, vn->v_vfs);
        return 0;
    }
    en = en->child;
    while(en) {
        if(!pc_cmp(path, en->name))
            break;
        en = en->next;
    }
    if(en) {
        path->now+=str_len(en->name);
        _get_vnode(en, vpp, vn->v_vfs);
        return 0;
    }
    return -ENOENT;
}

int
mfs_getdents(vnode_t *vn, dirent_t *dents, int first, int count)
{
    if(vn->v_type != VNODE_TYPE_DIR)
        return -ENOTDIR;
    count /= sizeof(dirent_t);
    mfs_node_t *node = vn->v_private;
    node = node->child;
    int bcount = 0;
    while(node && first-- > 0)
        node = node->next;
    while(node && count>0) {
        dents->d_ino = (int)node;
        str_cpy(dents->d_name, node->name);
        dents++;
        count--;
        node = node->next;
        bcount += sizeof(dirent_t);
    }
    return bcount;
}


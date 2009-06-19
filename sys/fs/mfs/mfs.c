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
static vnode_inactive_t mfs_inactive;

static vnode_ops_t mfs_vnode_ops = {
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
    .vop_symlink = mfs_symlink,
    .vop_inactive = mfs_inactive,
};

static mfs_node_t* _alloc_node(void);
static int mfs_from_image(mfs_data_t *mfs, unsigned char *image, int im_size);
static int _get_vnode(mfs_node_t *node, vnode_t **vpp, vfs_t *fs);
static int _create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr);
static int pc_cmp(lkp_state_t *path, const char *fname);


mfs_node_t*
_alloc_node()
{
    mfs_node_t *n = kmem_zalloc(sizeof(mfs_node_t), KM_SLEEP);
    n->vnode = NULL;
    return n;
}

int
mfs_open(vnode_t *vn, int flags, mode_t mode)
{
    return 0;
}

int
_create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    *vpp = NULL;
    if(!vn || !name || !attr)
        return -EINVAL;
    if(vn->v_type != VNODE_TYPE_DIR)
        return -ENOTDIR;
    mfs_node_t *pnode = vn->v_private;
    mfs_node_t *node = _alloc_node();
    if(!node)
        return -ENOMEM;
    node->name = str_dup(name);
    node->size = attr->va_size;
    node->type = (attr->va_type==VNODE_TYPE_REG)?MFS_TYPE_REG:
                 (attr->va_type==VNODE_TYPE_DIR)?MFS_TYPE_DIR:
                 (attr->va_type==VNODE_TYPE_LNK)?MFS_TYPE_LNK:0;
    node->attr = attr->va_mode;
    node->data = (attr->va_size>0)?kmem_alloc(attr->va_size, KM_SLEEP):NULL;
    if(node->data == NULL)
        node->size = 0;
    node->parent = pnode;
    node->child = NULL;
    node->next = pnode->child;
    pnode->child = node;
    return _get_vnode(node, vpp, vn->v_vfs);
}

int
mfs_create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    return _create(vn, vpp, name, attr);
}

int
mfs_mkdir(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    return _create(vn, vpp, name, attr);
}

int
mfs_close(vnode_t *vn)
{
    return 0;
}

int
mfs_read(vnode_t *vn, uio_t *u, int flags)
{
    mfs_node_t *node = vn->v_private;
    off_t start = u->offset;
    if(node->size < start)
        return -1;
    size_t size = MIN(node->size-start, u->size);
    u->resid = u->size;
    uio_move(node->data+start, size, u);
    return size;
}

int
mfs_write(vnode_t *vn, uio_t *u, int flags)
{
    if(!vn || vn->v_type != VNODE_TYPE_REG)
        return -EINVAL;
    return -ENOTSUP;
    return 0;
}

int
mfs_ioctl(vnode_t *vn, int cmd, uintptr_t arg)
{
    return -ENOTSUP;
}

int
mfs_seek(vnode_t *vn, off_t off)
{
    if(!vn)
        return -EINVAL;
    mfs_node_t *n = vn->v_private;
    if(n->size > off)
        return 0;
    return -EINVAL;
}

int
mfs_truncate(vnode_t *vn, off_t off)
{
    return -ENOTSUP;
}

int
mfs_getattr(vnode_t *vn, vattr_t *attr)
{
    mfs_node_t *node = vn->v_private;
    if(attr->va_mask & VATTR_SIZE)
        attr->va_size = node->size;
    if(attr->va_mask & VATTR_TYPE)
        attr->va_type = (node->type==MFS_TYPE_DIR)? VNODE_TYPE_DIR:
                        (node->type==MFS_TYPE_REG)? VNODE_TYPE_REG:
                        (node->type==MFS_TYPE_LNK)? VNODE_TYPE_LNK:0;
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
        if(attr->va_type == VNODE_TYPE_DIR)
            node->type = MFS_TYPE_DIR;
        else if(attr->va_type == VNODE_TYPE_REG)
            node->type = MFS_TYPE_REG;
        else if(attr->va_type == VNODE_TYPE_LNK)
            node->type = MFS_TYPE_LNK;
        else return -EINVAL;
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
    int err = _create(v, &vn, name, &va);
    if(err)
        return err;
    mfs_node_t *n = vn->v_private;
    mem_cpy(n->data, dst, n->size);
    vrele(vn);
    return 0;
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
mfs_getdents(vnode_t *vn, dirent_t *dents, int count)
{
    if(!vn)
        return -EINVAL;
    if(vn->v_type != VNODE_TYPE_DIR)
        return -ENOTDIR;
    count /= sizeof(dirent_t);
    mfs_node_t *node = vn->v_private;
    node = node->child;
    int bcount = 0;
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



struct mfs_data {
    vnode_t    *rootvnode;
    mfs_node_t *rootinode;
//    char       *rawdata;
};

void
mfs_init()
{
    vfs_register("mfs", &mfs_ops);
}


int
mfs_from_image(mfs_data_t *mfs, unsigned char *image, int im_size)
{
    mfs_header_t *header = (mfs_header_t*)image;
    if(header->magic0 != MFS_MAGIC0 || header->magic1 != MFS_MAGIC1) {
        DEBUGF("bad MFS image magic");
        return -1;
    }
    int ncount = header->items;
    mfs_node_t *nptr[ncount];
    for(int i=0; i<ncount; i++)
        nptr[i] = _alloc_node();

    mfs_data_entry_t *data = (mfs_data_entry_t*)(image + sizeof(mfs_header_t));
    for(int i=0; i<ncount; i++) {
        nptr[i]->name = str_dup(data->name);
        nptr[i]->size = data->size;
        nptr[i]->type = data->type;
        nptr[i]->attr = data->attr;
        nptr[i]->uid = 0;   // w obrazie nie mamy informacji o uid ani gid
        nptr[i]->gid = 0;
        nptr[i]->data = image + data->data_off;
        nptr[i]->parent = data->parent_id? nptr[data->parent_id-1]: NULL;
        nptr[i]->child = data->child_id? nptr[data->child_id-1]: NULL;
        nptr[i]->next = data->next_id? nptr[data->next_id-1]: NULL;
        data++;
//        kprintf("--Ent: %s\n", nptr[i]->name);
    }
    mfs->rootinode = nptr[0];
    return 0;
}


int
mfs_mount(vfs_t *fs)
{
    vnode_t *dv = NULL;
    tmp_vnode_dev(fs->vfs_mdev, &dv);
    if(!dv) {
        DEBUGF("cannot open dev");
        return -1;
    }
    mfs_data_t *mfs = kmem_zalloc(sizeof(mfs_data_t), KM_SLEEP);
    mfs->rootvnode = NULL;
    mfs->rootinode = NULL;
    fs->vfs_private = mfs;
    //jednak bez md chwilowo...
    extern unsigned char image[];
    extern unsigned int image_size;
    if(mfs_from_image(mfs, image, image_size))
        return -1;
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



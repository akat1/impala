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

#include <sys/types.h>
#include <sys/utils.h>
#include <sys/vfs.h>
#include <fs/devfs/devfs.h>
#include <fs/devfs/devfs_internal.h>
#include <sys/kmem.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/uio.h>

vfs_init_t           devfs_init;
static vfs_mount_t   devfs_mount;
static vfs_unmount_t devfs_unmount;
static vfs_sync_t    devfs_sync;
static vfs_getroot_t devfs_getroot;

static vfs_ops_t devfs_ops = {
    .vfs_mount = devfs_mount,
    .vfs_unmount = devfs_unmount,
    .vfs_getroot = devfs_getroot,
    .vfs_sync = devfs_sync
};

static vnode_open_t devfs_open;
static vnode_create_t devfs_create;
static vnode_close_t devfs_close;
static vnode_read_t devfs_read;
static vnode_write_t devfs_write;
static vnode_ioctl_t devfs_ioctl;
static vnode_seek_t devfs_seek;
static vnode_truncate_t devfs_truncate;
static vnode_getattr_t devfs_getattr;
static vnode_setattr_t devfs_setattr;
static vnode_lookup_t devfs_lookup;
static vnode_mkdir_t devfs_mkdir;
static vnode_getdents_t devfs_getdents;
static vnode_inactive_t devfs_inactive;
static vnode_readlink_t devfs_readlink;
static vnode_symlink_t devfs_symlink;
static vnode_access_t devfs_access;

static vnode_ops_t devfs_vnode_ops = {
    .vop_open = devfs_open,
    .vop_create = devfs_create,
    .vop_close = devfs_close,
    .vop_read = devfs_read,
    .vop_write = devfs_write,
    .vop_ioctl = devfs_ioctl,
    .vop_seek = devfs_seek,
    .vop_truncate = devfs_truncate,
    .vop_getattr = devfs_getattr,
    .vop_setattr = devfs_setattr,
    .vop_lookup = devfs_lookup,
    .vop_mkdir = devfs_mkdir,
    .vop_getdents = devfs_getdents,
    .vop_readlink = devfs_readlink,
    .vop_symlink = devfs_symlink,
    .vop_access = devfs_access,
    .vop_inactive = devfs_inactive,
};

static int _get_vnode(devfs_node_t *node, vnode_t **vpp, vfs_t *fs);
static int _create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr);
///@todo można by pewno wydzielić tę funkcję gdzieś
static int pc_cmp(lkp_state_t *path, const char *fname);


int
_get_vnode(devfs_node_t *node, vnode_t **vpp, vfs_t *fs)
{
    *vpp = NULL;
    if(node->i_type == _INODE_TYPE_DIR) {
        if(!node->i_dirvnode) {
            vnode_t *res = vnode_alloc();
            if(!res)
                return -ENOMEM;
            res->v_vfs = fs;
            res->v_flags = (node->i_parent)?0:VNODE_FLAG_ROOT;
            res->v_ops = &devfs_vnode_ops;
            res->v_type = VNODE_TYPE_DIR;
            res->v_dev = NULL;
            res->v_private = node;
            node->i_dirvnode = res;
        } else vref(node->i_dirvnode);
        *vpp = node->i_dirvnode;
        return 0;
    }
    //każdy dev własny vnode.. myślę, że to się przyda, jak nie to zmiana

    vnode_t *res = vnode_alloc();
    if(!res)
        return -ENOMEM;
    res->v_vfs = fs;
    res->v_flags = 0;
    res->v_ops = &devfs_vnode_ops;
    res->v_type = VNODE_TYPE_DEV;
    res->v_dev = node->i_dev;
    res->v_private = node;
    *vpp = res;
    return 0;
}

int
devfs_open(vnode_t *vn, int flags, mode_t mode)
{
    if(!vn->v_dev)
        return 0;
    return devd_open(vn->v_dev, flags); //zgodność flag?
}


int
_create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    int error;
    *vpp = NULL;
    if(!vn || !name || !attr || vn->v_type != VNODE_TYPE_DIR)
        return -EINVAL;
    ///@todo check for invalid name
    //check
    vnode_t *tmp;
    ///@todo w całym VFS i fs-ach trzeba przemyśleć strategię blokowania...
    error = vfs_lookup(vn, &tmp, name, NULL, LKP_NORMAL);
    if(error != -ENOENT) {
        if(!error)
            vrele(tmp);
        return -EEXIST; // a może raczej powinniśmy zmodyfikować ist. plik?
    }
    //plik nie istnieje -> tworzymy
    devfs_node_t *pn = vn->v_private;
    devfs_node_t *n = kmem_zalloc(sizeof(devfs_node_t), KM_SLEEP);
    if(!n)
        return -ENOMEM;

    strcpy(n->i_name, name);
    n->i_type = (attr->va_type == VNODE_TYPE_DEV)?
                            _INODE_TYPE_DEV:_INODE_TYPE_DIR;
    n->i_attr = attr->va_mode;
    n->i_uid = attr->va_uid;
    n->i_gid = attr->va_gid;
    n->i_dirvnode = NULL;
    n->i_dev = attr->va_dev;
    n->i_parent = pn;
    n->i_child = NULL;
    n->i_next = pn->i_child;
    pn->i_child = n;
    return _get_vnode(n, vpp, vn->v_vfs);
}

int
devfs_create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    return _create(vn, vpp, name, attr);
}

int
devfs_mkdir(vnode_t *vn, vnode_t **vpp, const char *path, vattr_t *attr)
{
    return _create(vn, vpp, path, attr);
}

int
devfs_close(vnode_t *vn)
{
    if(!vn->v_dev)
        return 0;
    return devd_close(vn->v_dev);
}

int
devfs_read(vnode_t *vn, uio_t *u, int flags)
{
    if(!vn->v_dev)
        return 0;
    return devd_read(vn->v_dev, u, flags);
}

int
devfs_write(vnode_t *vn, uio_t *u, int flags)
{
    if(!vn->v_dev)
        return 0;
    return devd_write(vn->v_dev, u, flags);
}

int
devfs_ioctl(vnode_t *vn, int cmd, uintptr_t arg)
{
    if(!vn->v_dev)
        return 0;
    return devd_ioctl(vn->v_dev, cmd, arg);
}

int
devfs_seek(vnode_t *vn, off_t off)
{
    if(!vn)
        return -EINVAL;
    return 0;
}

int
devfs_truncate(vnode_t *vn, off_t off)
{
    return -ENOTSUP;
}

int
devfs_getattr(vnode_t *vn, vattr_t *attr)
{
    devfs_node_t *node = vn->v_private;
    if(attr->va_mask & VATTR_SIZE)
        attr->va_size = 0;
    if(attr->va_mask & VATTR_TYPE)
        attr->va_type = (node->i_type==_INODE_TYPE_DIR)?
                            VNODE_TYPE_DIR:VNODE_TYPE_DEV;
    if(attr->va_mask & VATTR_MODE)
        attr->va_mode = node->i_attr;
    if(attr->va_mask & VATTR_UID)
        attr->va_uid = node->i_uid;
    if(attr->va_mask & VATTR_GID)
        attr->va_gid = node->i_gid;
    if(attr->va_mask & VATTR_DEV)
        attr->va_dev = node->i_dev;
    if (attr->va_mask & VATTR_NLINK)
        attr->va_nlink = 1;
    if (attr->va_mask & VATTR_INO)
        attr->va_ino = (ino_t) node;
    return 0;
}

int
devfs_setattr(vnode_t *vn, vattr_t *attr)
{
    devfs_node_t *node = vn->v_private;
    if(attr->va_mask & VATTR_SIZE) {
        return -EINVAL;
    }
    if(attr->va_mask & VATTR_TYPE) {    //eee... a może raczej zawsze -EINVAL..
        if(attr->va_type == VNODE_TYPE_DIR)
            node->i_type = _INODE_TYPE_DIR;
        else if(attr->va_type == VNODE_TYPE_DEV)
            node->i_type = _INODE_TYPE_DEV;
        else return -EINVAL;
    }
    if(attr->va_mask & VATTR_UID)
        node->i_uid = attr->va_uid;
    if(attr->va_mask & VATTR_GID)
        node->i_gid = attr->va_gid;
    if(attr->va_mask & VATTR_MODE)
        node->i_attr = attr->va_mode;
    if(attr->va_mask & VATTR_DEV)
        vn->v_dev = node->i_dev = attr->va_dev;
    return 0;
}

int
devfs_readlink(vnode_t *v, char *buf, int bsize)
{
    return -ENOTSUP;
}

int
devfs_symlink(vnode_t *v, char *name, char *dst)
{
    return -ENOTSUP;
}

int
devfs_access(vnode_t *v, int mode, pcred_t *c)
{
    if(!c)
        return 0;
    devfs_node_t *n = v->v_private;
    return vnode_access_ok(n->i_uid, n->i_gid, n->i_attr&0777, mode, c);
}

int
devfs_inactive(vnode_t *vn)
{
    devfs_node_t *n = vn->v_private;
    //nie zwalniamy n, on sobie bytuje aż do deregister
    if(vn->v_type == VNODE_TYPE_DIR) {
        n->i_dirvnode = NULL;
    }
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
devfs_lookup(vnode_t *vn, vnode_t **vpp, lkp_state_t *path)
{
    *vpp = NULL;
    devfs_node_t *en = vn->v_private;
    if(!pc_cmp(path, "..")) {
        path->now+=2;
        if(!en->i_parent) {
            *vpp = vn;
            vref(vn);
            return 0;
        }
        en = en->i_parent;
        return _get_vnode(en, vpp, vn->v_vfs);
    }
    en = en->i_child;
    while(en) {
        if(!pc_cmp(path, en->i_name))
            break;
        en = en->i_next;
    }
    if(en) {
        path->now+=strlen(en->i_name);
        return _get_vnode(en, vpp, vn->v_vfs);
    }
    return -ENOENT;
}

int
devfs_getdents(vnode_t *vn, dirent_t *dents, int first, int count)
{
    if(!vn)
        return -EINVAL;
    count /= sizeof(dirent_t);
    devfs_node_t *node = vn->v_private;
    node = node->i_child;
    int bcount = 0;
    while(node && first-- > 0)
        node = node->i_next;
    while(node && count>0) {
        dents->d_ino = (int)node;
        strcpy(dents->d_name, node->i_name);
        dents++;
        count--;
        node = node->i_next;
        bcount += sizeof(dirent_t);
    }
    return bcount;
}

devfs_node_t *devfs_rootinode = NULL;
vfs_t *devfs_vfs = NULL;

//powiadomienie vfs o istnieniu tego fs
void
devfs_init()
{
    vfs_register("devfs", &devfs_ops);

    devfs_node_t *n = kmem_zalloc(sizeof(devfs_node_t), KM_SLEEP);
    strcpy(n->i_name, "<devfs root>"); // ciekawe, czy da radę jakoś to zobaczyć...
    n->i_type = _INODE_TYPE_DIR;
    n->i_attr = 0;
    n->i_uid = 0;
    n->i_gid = 0;
    n->i_attr = 0755;
    n->i_dirvnode = NULL;
    n->i_dev = NULL;
    n->i_parent = NULL;
    n->i_child = NULL;
    n->i_next = NULL;
    devfs_rootinode = n;
}

vnode_t *
devfs_rootvnode()
{
    if(!devfs_rootinode)
        return NULL;
    vnode_t *node;
    _get_vnode(devfs_rootinode, &node, devfs_vfs);
    return node;
}

int
devfs_register(devd_t *device, uid_t def_uid, gid_t def_gid,
                mode_t def_mode)
{
    //szybka ściema..
    vattr_t attr;
    attr.va_dev = device;
    attr.va_uid = def_uid;
    attr.va_gid = def_gid;
    attr.va_mode = def_mode;
    attr.va_type = VNODE_TYPE_DEV;
    vnode_t *tmp_node;
    _create(devfs_rootvnode(), &tmp_node, device->name, &attr);
    return 0;
}

int
devfs_unregister(const char *name)
{
    return 0;
}

int
devfs_mount(vfs_t *fs)
{
    devfs_data_t *devfs = kmem_zalloc(sizeof(devfs_data_t), KM_SLEEP);
    KASSERT(devfs!=NULL);
    devfs->rootvnode = devfs_rootvnode();
    if(!devfs_vfs) {
        devfs_vfs = fs;
        devfs->rootvnode->v_vfs = fs;
    }
    fs->vfs_private = devfs;
    return 0;
}

int
devfs_unmount(vfs_t *fs)
{
    return 0;
}

void
devfs_sync(vfs_t *fs)
{
}

vnode_t *
devfs_getroot(vfs_t *fs)
{
    devfs_data_t *devfs = fs->vfs_private;
    if(!devfs->rootvnode) {
        _get_vnode(devfs_rootinode, &(devfs->rootvnode), fs);
    } else vref(devfs->rootvnode);
    return devfs->rootvnode;
}



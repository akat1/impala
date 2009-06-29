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
#include <sys/utils.h>
#include <sys/vfs.h>
#include <fs/fifofs/fifofs.h>
#include <fs/fifofs/fifofs_internal.h>
#include <sys/kmem.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/uio.h>

/*
vfs_init_t           fifofs_init;
static vfs_mount_t   fifofs_mount;
static vfs_unmount_t fifofs_unmount;
static vfs_sync_t    fifofs_sync;
static vfs_getroot_t fifofs_getroot;

static vfs_ops_t fifofs_ops = {
    .vfs_mount = fifofs_mount,
    .vfs_unmount = fifofs_unmount,
    .vfs_getroot = fifofs_getroot,
    .vfs_sync = fifofs_sync
};
*/
static vnode_open_t fifofs_open;
static vnode_create_t fifofs_create;
static vnode_close_t fifofs_close;
static vnode_read_t fifofs_read;
static vnode_write_t fifofs_write;
static vnode_ioctl_t fifofs_ioctl;
static vnode_seek_t fifofs_seek;
static vnode_truncate_t fifofs_truncate;
static vnode_getattr_t fifofs_getattr;
static vnode_setattr_t fifofs_setattr;
static vnode_lookup_t fifofs_lookup;
static vnode_mkdir_t fifofs_mkdir;
static vnode_getdents_t fifofs_getdents;
static vnode_readlink_t fifofs_readlink;
static vnode_symlink_t fifofs_symlink;
static vnode_sync_t fifofs_node_sync;
static vnode_inactive_t fifofs_inactive;

static vnode_ops_t fifofs_vnode_ops = {
    .vop_open = fifofs_open,
    .vop_create = fifofs_create,
    .vop_close = fifofs_close,
    .vop_read = fifofs_read,
    .vop_write = fifofs_write,
    .vop_ioctl = fifofs_ioctl,
    .vop_seek = fifofs_seek,
    .vop_truncate = fifofs_truncate,
    .vop_getattr = fifofs_getattr,
    .vop_setattr = fifofs_setattr,
    .vop_lookup = fifofs_lookup,
    .vop_mkdir = fifofs_mkdir,
    .vop_getdents = fifofs_getdents,
    .vop_readlink = fifofs_readlink,
    .vop_symlink = fifofs_symlink,
    .vop_sync    = fifofs_node_sync,
    .vop_inactive = fifofs_inactive,
};

static int _get_read_vnode(fifofs_node_t *node, vnode_t **vpp);
static int _get_write_vnode(fifofs_node_t *node, vnode_t **vpp);

//static int _create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr);

int
_get_read_vnode(fifofs_node_t *node, vnode_t **vpp)
{
    *vpp = NULL;
    if(!node->i_readnode) {
        vnode_t *res = vnode_alloc();
        if(!res)
            return -ENOMEM;
        res->v_vfs = NULL;
        res->v_flags = 0;
        res->v_ops = &fifofs_vnode_ops;
        res->v_type = VNODE_TYPE_FIFO;
        res->v_dev = NULL;
        res->v_private = node;
        node->i_readnode = res;
    } else vref(node->i_readnode);
    *vpp = node->i_readnode;
    return 0;
}

int
_get_write_vnode(fifofs_node_t *node, vnode_t **vpp)
{
    *vpp = NULL;
    if(!node->i_writenode) {
        vnode_t *res = vnode_alloc();
        if(!res)
            return -ENOMEM;
        res->v_vfs = NULL;
        res->v_flags = 0;
        res->v_ops = &fifofs_vnode_ops;
        res->v_type = VNODE_TYPE_FIFO;
        res->v_dev = NULL;
        res->v_private = node;
        node->i_writenode = res;
    } else vref(node->i_writenode);
    *vpp = node->i_writenode;
    return 0;
}

int
fifofs_open(vnode_t *vn, int flags, mode_t mode)
{
    return -ENOTSUP;
}

/*
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
    ///@todo w ca³ym VFS i fs-ach trzeba przemy¶leæ strategiê blokowania...
    error = vfs_lookup(vn, &tmp, name, NULL, LKP_NORMAL);
    if(error != -ENOENT) {
        vrele(tmp);
        return -EEXIST; // a mo¿e raczej powinni¶my zmodyfikowaæ ist. plik?
    }
    //plik nie istnieje -> tworzymy
    fifofs_node_t *pn = vn->v_private;
    fifofs_node_t *n = kmem_zalloc(sizeof(fifofs_node_t), KM_SLEEP);
    str_cpy(n->i_name, name);
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
*/
int
fifofs_create(vnode_t *vn, vnode_t **vpp, const char *name, vattr_t *attr)
{
    return -ENOTSUP;
}

int
fifofs_mkdir(vnode_t *vn, vnode_t **vpp, const char *path, vattr_t *attr)
{
    return -ENOTSUP;
}

int
fifofs_close(vnode_t *vn)
{
    fifofs_node_t *n = vn->v_private;
    char *which = (n->i_readnode == vn) ? "read": "write";
    kprintf("curthread: 0x%08x\n", curthread);
    DEBUGF("closing pipe 0x%08x- %s end fd: in=%u out=%u\n", vn, which, n->stat_in,
            n->stat_out);
    return 0;
}

int
fifofs_read(vnode_t *vn, uio_t *u, int flags)
{
    fifofs_node_t *n = vn->v_private;
    clist_t volatile *l = n->i_buf;
    KASSERT(l && l->buf!=NULL);
    int size = l->size;
    int want = u->resid = ISSET(flags, O_NONBLOCK) ?
                        min(u->size, size) : u->size;
    while(u->resid > 0) {
        char *read_beg = &l->buf[l->end];
        
        size_t read_size= min(min((l->buf_size - l->end), u->resid), l->size);
        if(read_size == 0) {
            if(!n->i_writenode) {
                if(l->size > 0)
                    continue;
                return want-u->resid;
            }
            sched_yield();
            continue;   //busy waiting... naprawiæ..
        }
        uio_move(read_beg, read_size, u);
        l->end += read_size;
        if(l->end == l->buf_size)
            l->end = 0;
        l->size -= read_size;
    }
    n->stat_out += want;
    return want;
}

int
fifofs_write(vnode_t *vn, uio_t *u, int flags)
{
    fifofs_node_t *n = vn->v_private;
    clist_t *l = n->i_buf;
    KASSERT(l && l->buf!=NULL);
    int size = l->buf_size - l->size;
    if(ISSET(flags, O_NONBLOCK) && u->size <= PIPE_BUF && size < u->size)
        return -EAGAIN;
    int want= u->resid = ISSET(flags, O_NONBLOCK)? min(u->size, size) : u->size;
    while(u->resid > 0) {
        char *write_beg = &l->buf[l->beg];
        size_t write_size =
            min(min(l->buf_size - l->beg, u->resid), l->buf_size - l->size);
        if(write_size == 0) {
            if(!n->i_readnode) {
                //signal(...., SIGPIPE);
            }
            sched_yield();
            continue;
        }
        uio_move(write_beg, write_size, u);
        l->beg += write_size;
        if(l->beg == l->buf_size)
            l->beg = 0;
        l->size += write_size;
    }
    n->stat_in += want;
    return want;
}

int
fifofs_ioctl(vnode_t *vn, int cmd, uintptr_t arg)
{
    return -ENOTSUP;
}

int
fifofs_seek(vnode_t *vn, off_t off)
{
    return -ENOTSUP;
}

int
fifofs_truncate(vnode_t *vn, off_t off)
{
    return -ENOTSUP;
}

int
fifofs_getattr(vnode_t *vn, vattr_t *attr)
{
    return -ENOTSUP;
}

int
fifofs_setattr(vnode_t *vn, vattr_t *attr)
{
    return -ENOTSUP;
}

int
fifofs_readlink(vnode_t *v, char *buf, int bsize)
{
    return -ENOTSUP;
}

int
fifofs_symlink(vnode_t *v, char *name, char *dst)
{
    return -ENOTSUP;
}

int
fifofs_node_sync(vnode_t *vn)
{
    return 0;
}

int
fifofs_inactive(vnode_t *vn)
{
    fifofs_node_t *n = vn->v_private;
//    DEBUGF("inactiving pipe channe: in=%u out=%u delta=%u, szleft=%u\n",
//            n->stat_in, n->stat_out, n->stat_in - n->stat_out, n->i_buf->size);
    // zwolnione vnode do odczytu lub zapisu, ale nie koniecznie oba
    if(vn == n->i_readnode)
        n->i_readnode = NULL;
    else if(vn == n->i_writenode)
        n->i_writenode = NULL;
    else return 0;
    if(!n->i_writenode && !n->i_readnode) {   //nie ma ¿adnych vnode'ów
        KASSERT(n->i_buf);
        clist_destroy(n->i_buf);
        n->i_buf = NULL;
        kmem_free(n);
    }
    return 0;
}

int
fifofs_lookup(vnode_t *vn, vnode_t **vpp, lkp_state_t *path)
{
    return -ENOTSUP;
}

int
fifofs_getdents(vnode_t *vn, dirent_t *dents, int first, int count)
{
    return -ENOTSUP;
}

//powiadomienie vfs o istnieniu tego fs
// void
// fifofs_init()
// {
//     vfs_register("fifofs", &fifofs_ops);
// }

int
fifo_create(vnode_t **re, vnode_t **we)
{
    int err = -ENOMEM;
    vnode_t *v1, *v2;
    fifofs_node_t *n = kmem_alloc(sizeof(fifofs_node_t), KM_SLEEP);
    n->stat_in = n->stat_out = 0;
    if(!n)
        return err;
    n->i_buf = clist_create(FIFO_SIZE);
    if(!n->i_buf)
        goto end_err0;
    n->i_readnode = n->i_writenode = NULL;
    if((err = _get_read_vnode(n, &v1)))
        goto end_err1;
    if((err = _get_write_vnode(n, &v2)))
        goto end_err2;
    *re = v1;
    *we = v2;
    return 0;

end_err2:
    vrele(v1);
end_err1:
    clist_destroy(n->i_buf);
end_err0:
    kmem_free(n);
    return err;
}
/*
int
fifofs_mount(vfs_t *fs)
{
    return -ENOTSUP;
}

int
fifofs_unmount(vfs_t *fs)
{
    return 0;
}

void
fifofs_sync(vfs_t *fs)
{
}

vnode_t *
fifofs_getroot(vfs_t *fs)
{
    return NULL;
}

*/

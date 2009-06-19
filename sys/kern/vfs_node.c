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
#include <sys/vfs.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <sys/uio.h>
#include <sys/thread.h>
#include <sys/proc.h>
#include <sys/utils.h>
#include <fs/devfs/devfs.h>

//#include <sys/utils.h>

static bool _last_component(lkp_state_t *st);
static int _rdwr(int space, int rw, vnode_t *vn, void *addr, int len,
                  off_t offset);

vnode_t*
vnode_alloc()
{
    vnode_t *res = kmem_alloc(sizeof(vnode_t), KM_SLEEP);
    res->v_vfs_mounted_here = NULL;
    res->v_refcnt=1;
    return res;
}

void
vref(vnode_t *vn)
{
//     DEBUGF("VREF vn=%p", vn);
    KASSERT(vn->v_refcnt>0);
    vn->v_refcnt++;
}

void
vrele(vnode_t *vn)
{
//     DEBUGF("VRELE vn=%p", vn);
    KASSERT(vn->v_refcnt>0);
    vn->v_refcnt--;
    if(vn->v_refcnt == 0) {
        VOP_INACTIVE(vn);
        kmem_free(vn);
    }
}

int
vnode_opendev(const char *devname, int mode, vnode_t **vn)
{
    vnode_t *tmp;
    *vn = NULL;
    int error = vfs_lookup(devfs_rootvnode(), &tmp, devname, NULL, LKP_NORMAL);
    if(error)
        return error;
    if(tmp->v_type != VNODE_TYPE_DEV)
        return -ENODEV;
    error = VOP_OPEN(tmp, 0, mode);
    if(error)
        return error;
    *vn = tmp;
    return 0;
}

bool
vnode_isatty(vnode_t *vn)
{
    if(!vn || vn->v_type != VNODE_TYPE_DEV)
        return FALSE;
    if(vn->v_dev->type == DEV_TTY)
        return TRUE;
    return FALSE;
}

int
vfs_lookup(vnode_t *sd, vnode_t **vpp, const char *p, thread_t *thr, int f)
{
    *vpp = NULL;
    if(!p) return -EINVAL;
    lkp_state_t pc;
    pc.flags = f;
    pc.max_link_cnt = 10;
    pc.path = p;
    pc.now = p;
    return vfs_lookupcp(sd, vpp, &pc, thr);
}

// mo¿na to jako¶ zgrupowaæ... np. wzoruj±c siê na namei

int
vfs_lookup_parent(vnode_t *sd, vnode_t **vpp, const char *p, thread_t *thr)
{
    *vpp = NULL;
    if(!p) return -EINVAL;
    lkp_state_t pc;
    pc.flags = LKP_GET_PARENT;
    pc.max_link_cnt = 10;
    pc.path = p;
    pc.now = p;
    return vfs_lookupcp(sd, vpp, &pc, thr);
}

bool
_last_component(lkp_state_t *st)
{
    const char *cur = st->now;
    while(*cur) {
        if(*(cur++) == '/')
            return FALSE;
    }
    return TRUE;
}

static void _change_vptr(vnode_t **v, vnode_t *new);

void
_change_vptr(vnode_t **v, vnode_t *new)
{
    if(*v == new)
        return;
    if(new)
        vref(new);
    if(*v)
        vrele(*v);
    *v = new;
}

int
vfs_lookupcp(vnode_t *sd, vnode_t **vpp, lkp_state_t *path, thread_t *thr)
{
    int errno = 0;
    vnode_t *tmp;
    *vpp = NULL;
    if(!sd)
        sd = rootvnode;

    vnode_t *cur = sd, *last = sd;
    vref(last); vref(cur);
    if(*(path->now) == '/') {
        (path->now)++;
        if(thr)
            _change_vptr(&cur, thr->thr_proc->p_rootdir);
        else
            _change_vptr(&cur, rootvnode);
    }
    if(!cur) {
        vrele(cur); vrele(last);
        return -ENOENT;
    }
    if(cur->v_type != VNODE_TYPE_DIR) {
        vrele(cur); vrele(last);
        return -ENOTDIR;
    }
    if(cur->v_vfs_mounted_here) {
            tmp = VFS_GETROOT(cur->v_vfs_mounted_here);
            vrele(cur);
            cur = tmp;
    }

    while(*(path->now)) {
        if(*(path->now) == '/') {
            (path->now)++;
            continue;
        }
        if(*(path->now) == '.') {
            (path->now)++;
            if(*(path->now) == '.') {
                (path->now)++;
                if(*(path->now) == '/') {
                    //idziemy w górê..
                    if(cur->v_flags & VNODE_FLAG_ROOT) {
                        tmp = cur->v_vfs->vfs_mpoint;
                        if(tmp!=NULL) {// to nie jest korzeñ
                            vrele(last);
                            last = cur;
                            cur = tmp;
                        }
                    }
                }
                (path->now)-=2;
            } else  //brak drugiej kropki
            if(*(path->now) == '/') { // zostajemy gdzie jeste¶my
                (path->now)++;
                continue;
            } else  // co¶ innego z kropk±..
                (path->now)--;
        }
        errno = VOP_LOOKUP(cur, &tmp, path);  //niech vnode dalej szuka...
        if(errno == -ENOENT) {
            if((path->flags & LKP_GET_PARENT) && _last_component(path)) {
                vrele(last);
                last = cur;
                vref(cur);
                errno = 0;
            }
            break;
        } else if(errno)
            break;
        vrele(last);
        last = cur;
        cur = tmp;

        if(!(path->flags & LKP_NO_FOLLOW) || *(path->now))
            while(cur->v_type == VNODE_TYPE_LNK) {
                char lpath[PATH_MAX];
                errno = VOP_READLINK(cur, lpath, PATH_MAX);
                if(errno == PATH_MAX)
                    goto end_error;
                if(errno<=0) {
                    goto end_error;
                }
                lpath[errno] = 0;
                lkp_state_t pc;
                pc.flags = LKP_NORMAL;
                pc.max_link_cnt = --(path->max_link_cnt);
                pc.now = pc.path = lpath;
                errno = vfs_lookupcp(last, &tmp, &pc, thr);
                if(errno)
                    goto end_error;
                if((path->max_link_cnt = pc.max_link_cnt)<0) {
                    errno = -ELOOP;
                    goto end_error;
                }
                vrele(cur);
                cur = tmp;  //last nie aktualizujemy, zgadza siê? ;)

            }
        if(*(path->now) && (cur->v_type != VNODE_TYPE_DIR)) {
            errno = -ENOTDIR;
            goto end_error;
        }
        if(cur->v_vfs_mounted_here) {
            vrele(cur);
            cur = VFS_GETROOT(cur->v_vfs_mounted_here);
        }
    }
    if(errno)
        goto end_error;
    if(path->flags & LKP_GET_PARENT) {
        vrele(cur);
        *vpp = last;
    }
    else {
        vrele(last);
        *vpp = cur;
    }
    return 0;

end_error:
    if(cur)
        vrele(cur);
    if(last)
        vrele(last);
    return errno;
}


int
tmp_vnode_dev(devd_t *dev, vnode_t **vn)
{
    vnode_t *v = vnode_alloc();
    v->v_dev = dev;
    v->v_type = VNODE_TYPE_DEV;
    *vn = v;
    return 0;
}



int
_rdwr(int space, int rw, vnode_t *vn, void *addr, int len, off_t offset)
{
    if(!vn || !addr)
        return -EINVAL;
    uio_t u;
    iovec_t iov;
    iov.iov_base = addr;
    iov.iov_len = len;
    u.iovs = &iov;
    u.iovcnt = 1;
    u.size = len;
    u.offset = offset;
    u.oper = rw;
    u.space = space;
    u.resid = u.size;
    if(u.oper == UIO_WRITE)
        return VOP_WRITE(vn, &u, O_RDWR);
    else if(u.oper == UIO_READ)
        return VOP_READ(vn, &u, O_RDWR);
    else return -EINVAL;
}

int
vnode_rdwr(int rw, vnode_t *vn, void *addr, int len, off_t offset)
{
    return _rdwr(UIO_SYSSPACE, rw, vn, addr, len, offset);
}

int
vnode_urdwr(int rw, vnode_t *vn, void *addr, int len, off_t offset)
{
    return _rdwr(UIO_USERSPACE, rw, vn, addr, len, offset);
}

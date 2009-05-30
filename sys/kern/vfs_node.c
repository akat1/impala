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
#include <fs/devfs/devfs.h>

//#include <sys/utils.h>

static bool _last_component(lkp_state_t *st);

vnode_t*
vnode_alloc()
{
    vnode_t *res = kmem_alloc(sizeof(vnode_t), KM_SLEEP);
    res->v_refcnt=1;
    return res;
}

void
vref(vnode_t *vn)
{
    vn->v_refcnt++;
}

void
vrele(vnode_t *vn)
{
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
    int error = vfs_lookup(devfs_rootvnode(), &tmp, devname, NULL);
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

int
vfs_lookup(vnode_t *sd, vnode_t **vpp, const char *p, thread_t *thr)
{
    *vpp = NULL;
    if(!p) return -EINVAL;
    lkp_state_t pc;
    pc.flags = 0;
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

int
vfs_lookupcp(vnode_t *sd, vnode_t **vpp, lkp_state_t *path, thread_t *thr)
{
    int errno = 0;
    vnode_t *tmp;
    *vpp = NULL;
    vnode_t *cur = sd;
    if(*(path->now) == '/') {
        (path->now)++;
        if(thr)
            cur = thr->thr_proc->p_rootdir;
        else
            cur = rootvnode;
    }
    if(!cur)
        return -ENOENT;
    while(*(path->now)) {
        if(cur->v_type != VNODE_TYPE_DIR)
            return -ENOTDIR;
        if(cur->v_vfs_mounted_here) {
            cur = VFS_GETROOT(cur->v_vfs_mounted_here);
        }
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
                        if(tmp!=NULL) // to nie jest korzeñ
                            cur = tmp;
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
            if((path->flags & LKP_GET_PARENT) && _last_component(path))
                errno = 0;                
            break;
        } else if(errno)
            break;
        cur = tmp;            
    }
    if(errno)
        return errno;
    *vpp = cur;
    return 0;
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
vnode_rdwr(int rw, vnode_t *vn, void *addr, int len, off_t offset)
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
    u.space = UIO_SYSSPACE; //vn_rdwr bêdzie wykorzystywany gdzie¶ indziej?
    if(u.oper == UIO_WRITE)
        return VOP_WRITE(vn, &u);
    else if(u.oper == UIO_READ)
        return VOP_READ(vn, &u);
    else return -EINVAL;
}



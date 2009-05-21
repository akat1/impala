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


vnode_t*
vnode_alloc()
{
    vnode_t *res = kmem_alloc(sizeof(vnode_t), KM_SLEEP);
    res->v_refcnt++;
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
        kmem_free(vn);
    }
}

int
vnode_opendev(const char *devname, int mode, vnode_t **vn)
{
    devd_t *d = devd_find(devname);
    if (!d) return -ENODEV;
    int e = devd_open(d,mode);
    if (e == 0) {
        vnode_t *v = kmem_alloc( sizeof(*vn), KM_SLEEP );
        v->v_dev = d;
        v->v_type = VNODE_TYPE_DEV;
        *vn = v;
    }
    return e;
}


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
#include <sys/thread.h>
#include <sys/vfs.h>
#include <sys/kmem.h>
#include <sys/string.h>
#include <sys/utils.h>
#include <sys/fcntl.h>
#include <fs/mfs/mfs.h>

static void register_fss(void);
static bool is_this_fsname(const vfs_conf_t *conf, const char *known);

static list_t   filesystems;
static mutex_t  global_lock;

void
vfs_init()
{
    mutex_init(&global_lock, MUTEX_NORMAL);
    list_create(&filesystems, offsetof(vfs_conf_t,L_confs), FALSE);
    register_fss();
}

void
register_fss()
{
    fs_mfs_init();
}

void
vfs_register(const char *name, vfs_ops_t *ops)
{
    // nie sprawdzamy czy istnieje ju¿ taki system plików.
    mutex_lock(&global_lock);
    vfs_conf_t *vc = kmem_alloc( sizeof(*vc), KM_SLEEP);
    vc->name = name;
    vc->ops = ops;
    list_insert_tail(&filesystems, vc); 
    mutex_unlock(&global_lock);
}

vfs_conf_t *
vfs_byname(const char *name)
{
    vfs_conf_t *c;
    mutex_lock(&global_lock);
    c = list_find(&filesystems, (list_pred_f*)is_this_fsname, name);
    mutex_unlock(&global_lock);
    return c;
}

bool
is_this_fsname(const vfs_conf_t *conf, const char *known)
{
    return (str_cmp(conf->name,known)==0);
}


vfs_mp_t *
vfs_mp_create(vnode_t *dev_node, const char *fstype, const char *mpoint)
{
    vfs_conf_t *conf = vfs_byname(fstype);
    if (conf == NULL) return NULL;
    ///@TODO: VFS: sprawdzanie poprawno¶ci mpoint
    vfs_mp_t *mp = kmem_alloc(sizeof(*mp), KM_SLEEP);
    mp->dev_node = dev_node;
    mp->vfs_conf = conf;
    return mp;
}


void
vfs_mountroot()
{
    // Na sztywno wpisane mfs:/dev/md0
    DEBUGF("Trying to mount from mfs:/dev/md0");
    vnode_t *devn;
    if (vnode_opendev("md0", O_RDWR, &devn) != 0) {
        panic("cannot find root device");
    }
    vfs_mp_t *mp = vfs_mp_create(devn, "mfs", "/");
    if (!mp) {
        panic("cannot create root mount point");
    }
    if( VFS_MOUNT(mp) != 0 ) {
        panic("cannot mount file system");
    }
}

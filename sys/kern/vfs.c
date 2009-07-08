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
#include <sys/kargs.h>
#include <sys/kmem.h>
#include <sys/string.h>
#include <sys/utils.h>
#include <sys/fcntl.h>
#include <fs/mfs/mfs.h>
#include <fs/devfs/devfs.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <dev/md/md.h>

static void register_fss(void);
static bool is_this_fsname(const vfs_conf_t *conf, const char *known);

static list_t   filesystems;
static list_t   mounted_fs;
static mutex_t  global_lock;

vnode_t *rootvnode;

void
vfs_init()
{
    mutex_init(&global_lock, MUTEX_NORMAL);
    list_create(&filesystems, offsetof(vfs_conf_t, L_confs), FALSE);
    list_create(&mounted_fs, offsetof(vfs_t, L_mountlist), FALSE);
    register_fss();
}

int
vfs_getinfos(int off, struct mountinfo *tab, int n)
{
    int r = 0;
    MUTEX_LOCK(&global_lock, "mountinfo");
    int i;
    vfs_t *v = list_head(&mounted_fs);
    for (i = 0; i < off && v; i++, v = list_next(&mounted_fs, v));
    if (i == off && v != NULL) {
        for (i = 0; i < n && v; i++, v = list_next(&mounted_fs, v)) {
            str_cpy(tab[i].type, v->vfs_conf->name);
            str_cpy(tab[i].mpoint, "<unknown>");
            if (v->vfs_mdev)
                str_cpy(tab[i].dev, v->vfs_mdev->name);
                else str_cpy(tab[i].dev, "null");
        }
        r = i;
    }
    mutex_unlock(&global_lock);
    return r;
}

void
register_fss()
{
    extern vfs_init_t *fstab[];
    for (int i = 0; fstab[i] != NULL; i++) {
        (fstab[i])();
    }
}

void
vfs_register(const char *name, vfs_ops_t *ops)
{
    // nie sprawdzamy czy istnieje ju¿ taki system plików.
    MUTEX_LOCK(&global_lock, "vfs");
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
    MUTEX_LOCK(&global_lock, "vfs");
    c = list_find(&filesystems, is_this_fsname, name);
    mutex_unlock(&global_lock);
    return c;
}

bool
is_this_fsname(const vfs_conf_t *conf, const char *known)
{
    return (str_cmp(conf->name,known)==0);
}


int
vfs_create(vfs_t **fs, const char *fstype)
{
    *fs = NULL;
    vfs_conf_t *conf = vfs_byname(fstype);
    if (conf == NULL) return -EINVAL;
    vfs_t *rfs = kmem_alloc(sizeof(vfs_t), KM_SLEEP);
    if(!rfs)
        return -ENOMEM;
    rfs->vfs_private = NULL;
    rfs->vfs_mpoint = NULL;
    rfs->vfs_mdev = NULL;
    rfs->vfs_conf = conf;
    rfs->vfs_ops = conf->ops;
    *fs = rfs;
    return 0;
}


//tylko taki schemacik, jak to w przysz³o¶ci mo¿e wygl±daæ..
void
vfs_mountroot()
{
//     const char *_rootdev = "fd0";
//     // Na sztywno wpisane mfs:/dev/md0
//     vnode_t *devn = NULL;
// 
//     karg_get_s("rootdev", &_rootdev);
//     DEBUGF("trying to mount from fatfs:/dev/%s", _rootdev);
//     if (vnode_opendev(_rootdev, 0/*O_RDWR*/, &devn) != 0) {
//         panic("cannot open root device %s", _rootdev);
//     }

    vfs_t *fs;
    vfs_create(&fs, "mfs");
    if (!fs) {
        panic("cannot create root mount point");
    }
//    if(!devn) {
//        panic("Internal error - opendev succeded and devn NULL");
//    }
    fs->vfs_mdev = NULL;//devn->v_dev;
    fs->vfs_mpoint = NULL;  //montuj nigdzie -> twórz samodzielne drzewko
    if( VFS_MOUNT(fs) != 0 ) {
        panic("cannot mount file system");
    }
    rootvnode = VFS_GETROOT(fs);
    list_insert_tail(&mounted_fs, fs);
}


int
vfs_mount(const char *name, vnode_t *mpoint, devd_t *dev)
{
    vfs_t *fs;
    if(mpoint->v_type != VNODE_TYPE_DIR)
        return -ENOTDIR;
    vfs_create(&fs, name);
    if (!fs)
        return -1;
    fs->vfs_mdev = dev;
    vref(mpoint);   //niech mpoint bêdzie nasz
    fs->vfs_mpoint = mpoint;
    if( VFS_MOUNT(fs) != 0 ) {
        panic("cannot mount file system");
    }
    VFS_GETROOT(fs);
    //lock
    mpoint->v_vfs_mounted_here = fs;
    list_insert_tail(&mounted_fs, fs);
    //unlock
    return 0;
}


/*
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
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vfs.h>

typedef struct mkdir_args mkdir_args_t;
struct mkdir_args {
    const char *name;
    mode_t m;
};

int sc_mkdir(thread_t *t, syscall_result_t *r, mkdir_args_t *args);

int
sc_mkdir(thread_t *t, syscall_result_t *r, mkdir_args_t *args)
{
    vnode_t *vp;
    int err = 0;
    char path[PATH_MAX];
    if((err = copyinstr(path, args->name, PATH_MAX)))
        return err;
    proc_t *p = t->thr_proc;
    vnode_t *parent;
    if((err = vfs_lookup_parent(p->p_curdir, &parent, path, t))) {
        return err;
    }
    if((err = VOP_ACCESS(parent, W_OK, p->p_cred))) {
        vrele(parent);
        return err;
    }
    char *bname = path;
    for(int i=str_len(path)-1; i>=0; i--) {
        if(path[i]!='/')
            break;
        else
            path[i]='\0';
    }
    for(int i=0; i<PATH_MAX; i++) {
        if(!path[i])
            break;
        if(path[i] == '/')
            bname = &path[i+1];
    }
    vattr_t a;
    mem_zero(&a, sizeof(a));
    a.va_type = VNODE_TYPE_DIR;
    a.va_mode = args->m & ~(p->p_umask) & 0777;
    a.va_uid = p->p_cred->p_uid;
    a.va_gid = p->p_cred->p_gid;
    if((err = VOP_MKDIR(parent, &vp, bname, &a)))
        return err;
    vrele(vp);
    vrele(parent);
    return 0;
}



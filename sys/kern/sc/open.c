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

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/utils.h>
#include <sys/syscall.h>
#include <sys/vfs.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/string.h>
#include <sys/vm.h>

static const char * _get_last_cmpt(const char *path);

const char *
_get_last_cmpt(const char *path)
{
    int l=str_len(path);
    for(const char *r = path+l-1; r>=path; r--) {
        if(*r == '/')
            return r+1;
    }
    return path;
}


typedef struct sc_open_args sc_open_args;

struct sc_open_args {
    addr_t fname;
    int flags;
    mode_t mode;
};

errno_t sc_open(thread_t *p, syscall_result_t *r, sc_open_args *arg);

errno_t
sc_open(thread_t *p, syscall_result_t *r, sc_open_args *arg)
{
    int fd;
    int flags = arg->flags;
    int error = 0;
    vnode_t *node;
    file_t  *file;
    proc_t *proc = p->thr_proc;
    if((error = vm_validate_string(arg->fname, 256)))
        return error;
    KASSERT(proc->p_rootdir!=NULL);
    kprintf("fname: %p, flags: %u, mode: %u\n", arg->fname, arg->flags,
             arg->mode);
    error = vfs_lookup(proc->p_curdir, &node, arg->fname, p, LKP_NORMAL);
    if(error) {
        if(!(flags & O_CREAT))
            return error;
        vnode_t *parent;
        error = vfs_lookup_parent(proc->p_curdir, &parent, arg->fname, p);
        if(error)
            return error;
        vattr_t attr;
        attr.va_mode = arg->mode & ~(proc->p_umask);
        attr.va_type = VNODE_TYPE_REG;
        attr.va_uid = proc->p_cred->p_uid;
        attr.va_gid = proc->p_cred->p_gid;
        attr.va_size = 0; attr.va_dev = NULL;
        error = VOP_CREATE(parent, &node, _get_last_cmpt(arg->fname), &attr);
        if(error)
            return error;
    } else {
        //plik istnieje
        if((flags & O_CREAT) && (flags & O_EXCL))
            return -EEXIST;
        if((node->v_type == VNODE_TYPE_DIR) && (flags & (O_RDWR | O_WRONLY)))
            return -EISDIR;
    }

    if((error = f_alloc(proc, node, &file, &fd)))
        return error;
    if((error = VOP_OPEN(node, arg->flags, arg->mode)))
        return error;
    file->f_flags = flags;
    r->result = fd;
    return -EOK;
}


/*
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

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/utils.h>
#include <sys/syscall.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/string.h>

typedef struct unlink_args unlink_args;

struct unlink_args {
    char *pathname;
};

errno_t sc_unlink(thread_t *t, syscall_result_t *r, unlink_args *args);

errno_t
sc_unlink(thread_t *t, syscall_result_t *r, unlink_args *args)
{
    int err = 0;
    char path[PATH_MAX];
    if((err = copyinstr(path, args->pathname, PATH_MAX)))
        return err;
    proc_t *p = t->thr_proc;
    vnode_t *parent;
    if((err = vfs_lookup_parent(p->p_curdir, &parent, path, t))) {
        return err;
    }
    char *bname = path;
    for(int i=strlen(path)-1; i>=0; i--) {
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
    int res = VOP_UNLINK(parent, bname);
    vrele(parent);
    if(res < 0)
        return res;
    r->result = res;
    return 0;
}


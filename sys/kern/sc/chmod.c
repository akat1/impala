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

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vm.h>

typedef struct chmod_args chmod_args_t;
struct chmod_args {
    char *fname;
    mode_t mode;
};

int sc_chmod(thread_t *p, syscall_result_t *r, chmod_args_t *args);

int
sc_chmod(thread_t *t, syscall_result_t *r, chmod_args_t *args)
{
    int res=0;
    proc_t *p = t->thr_proc;
    vnode_t *node;
    char pname[PATH_MAX];
    if((res = copyinstr(pname, args->fname, PATH_MAX)))
        return res;

    res = vfs_lookup(p->p_curdir, &node, pname, t, LKP_NORMAL);
    if(res)
        return res;
    vattr_t va;
    va.va_mask=VATTR_ALL;
    if((res = VOP_GETATTR(node, &va)))
        goto err;
    res = -EPERM;
    if(p->p_cred->p_euid != 0 && p->p_cred->p_euid != va.va_uid)
        goto err;
    mode_t mode = args->mode & 07777;
    if(ISSET(mode, S_ISGID) && p->p_cred->p_euid!=0 
        && va.va_gid!=p->p_cred->p_egid)
        UNSET(mode, S_ISGID); //mamy prawo kasować bit suid/sgid?
    va.va_mask = VATTR_MODE;
    va.va_mode = mode;
    if((res = VOP_SETATTR(node, &va)))
        goto err;
    vrele(node);    
    return 0;
err:
    vrele(node);
    return res;
}



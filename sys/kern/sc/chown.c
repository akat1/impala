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
 * $Id: TEMPLATE.c 486 2009-06-25 07:51:47Z wieczyk $
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/vm.h>

typedef struct chown_args chown_args_t;
struct chown_args {
    char *fname;
    uid_t uid;
    gid_t gid;
};

int sc_chown(thread_t *p, syscall_result_t *r, chown_args_t *args);

int
sc_chown(thread_t *t, syscall_result_t *r, chown_args_t *args)
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
    if(p->p_cred->p_euid != 0)  //tak... póki co nie ma zmian dla nie roota
        goto err;
    va.va_mask = VATTR_UID | VATTR_GID | VATTR_MODE;
    if(args->uid!=-1)
        va.va_uid = args->uid;
    if(args->gid!=-1)
        va.va_gid = args->gid;
    if(p->p_cred->p_euid!=0)   //tymczasowo niemożliwe
        UNSET(va.va_mode, S_ISUID | S_ISGID);
    if((res = VOP_SETATTR(node, &va)))
        goto err;
    vrele(node);    
    return 0;
err:
    vrele(node);
    return res;
}



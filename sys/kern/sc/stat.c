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
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/vm.h>

typedef struct stat_args stat_args;

struct stat_args {
    int          mode;
    char        *pathname;
    struct stat *buf;
};


errno_t sc_stat(thread_t *p, syscall_result_t *r, stat_args *args);

errno_t
sc_stat(thread_t *t, syscall_result_t *r, stat_args *args)
{
    int res=0;
    proc_t *p = t->thr_proc;
    vnode_t *node;
    char pname[PATH_MAX];
    if((res = copyinstr(pname, args->pathname, PATH_MAX)))
        return res;
    if((res = vm_is_avail((vm_addr_t)args->buf, sizeof(struct stat))))
        return res;
    res = vfs_lookup(p->p_curdir, &node, pname, t,
                      (args->mode == STAT_LINK)?LKP_NO_FOLLOW:LKP_NORMAL);
    if(res)
        return res;
    res = vnode_stat(node, args->buf);
    vrele(node);
    if(res)
        return res;
    return 0;
}


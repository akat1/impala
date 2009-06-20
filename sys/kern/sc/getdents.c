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
 * $Id: getdents.c 269 2009-05-26 12:45:39Z takeshi $
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/string.h>
#include <sys/syscall.h>
#include <machine/video.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/uio.h>

typedef struct sc_getdents_args sc_getdents_args;

struct sc_getdents_args {
    int fd;
    dirent_t *data;
    size_t size;
};


errno_t sc_getdents(thread_t *p, syscall_result_t *r, sc_getdents_args *args);

errno_t
sc_getdents(thread_t *t, syscall_result_t *r, sc_getdents_args *args)
{
    file_t *file = f_get(t->thr_proc->p_fd, args->fd);
    if (file == NULL) {
        return -EBADF;
    }
    vnode_t *v = file->f_vnode;
    int res = VOP_GETDENTS(v, args->data, file->f_offset, args->size);
    if(res > 0)
        file->f_offset += res/sizeof(dirent_t);
    frele(file);
    if(res < 0)
        return res;
    r->result = res;
    return -EOK;
}


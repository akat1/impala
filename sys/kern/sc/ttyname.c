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
#include <sys/vm.h>
#include <sys/device.h>

typedef struct ttyname_args ttyname_args_t;
struct ttyname_args {
    int fd;
    char *buf;
    size_t len;
};

int sc_ttyname(thread_t *p, syscall_result_t *r, ttyname_args_t *args);

int
sc_ttyname(thread_t *t, syscall_result_t *r, ttyname_args_t *args)
{
    int err = 0;
    int len = args->len;
    char *buf = args->buf;
    file_t *file = f_get(t->thr_proc->p_fd, args->fd);
    if (!file)
        return -EBADF;
    if((err = vm_is_avail((vm_addr_t)buf, len))) {
        frele(file);
        return err;
    }
    if(vnode_isatty(file->f_vnode) == FALSE) {
        frele(file);
        return -ENOTTY;
    }
    const char *name = file->f_vnode->v_dev->name;
    if(strlen(name) + 1 > len) {
        frele(file);
        return -ERANGE;
    }
//    strncpy(buf, name, len);
    snprintf(buf, len, "/dev/%s", name);
    frele(file);
    return 0;
}



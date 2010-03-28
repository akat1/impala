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
 * $Id: TEMPLATE.c 486 2009-06-25 07:51:47Z wieczyk $
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vm.h>
#include <sys/uio.h>
#include <sys/limits.h>

typedef struct writev_args writev_args_t;
struct writev_args {
    int fd;
    const iovec_t *vector;
    int count;
};

int sc_writev(thread_t *p, syscall_result_t *r, writev_args_t *args);

int
sc_writev(thread_t *t, syscall_result_t *r, writev_args_t *args)
{
    if(args->count < 0 || args->count > IOV_MAX)
        return -EINVAL;
    int err = 0;
    file_t *file = f_get(t->thr_proc->p_fd, args->fd);
    if (!file)
        return -EBADF;

    size_t vec_size = sizeof(iovec_t) * args->count;
    if((err = vm_is_avail((vm_addr_t)args->vector, vec_size)))
        goto error0;

    uio_t u;
    iovec_t *iovs = kmem_alloc(vec_size, KM_SLEEP);
    mem_cpy(iovs, args->vector, vec_size);
    size_t size = 0;
    for(int i=0; i<args->count; i++) {
        if((err = vm_is_avail((vm_addr_t)iovs[i].iov_base, iovs[i].iov_len)))
            goto error;
        size_t oldsize = size;
        size += iovs[i].iov_len;
        if(oldsize > size) {
            //Czy to starcza??
            err = -EINVAL;
            goto error;
        }
    }
    u.size = size;
    u.resid = u.size;
    u.iovs = iovs;
    u.iovcnt = args->count;
    u.oper = UIO_WRITE;
    u.space = UIO_SYSSPACE; //znowu ściema
    r->result = f_write(file, &u);
    kmem_free(iovs);
    frele(file);
    if(r->result < 0)
        return r->result;
    return 0;
error:
    kmem_free(iovs);
error0:
    frele(file);
    return err;
}

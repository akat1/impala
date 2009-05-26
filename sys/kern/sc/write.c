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
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/syscall.h>
#include <sys/console.h>
#include <machine/video.h>
#include <machine/interrupt.h>

typedef struct sc_write_args sc_write_args;

struct sc_write_args {
    int fd;
    addr_t data;
    size_t size;
};


errno_t sc_write(thread_t *t, syscall_result_t *r, sc_write_args *args);

errno_t
sc_write(thread_t *t, syscall_result_t *r, sc_write_args *args)
{
//    kprintf("write(%u,%p,%p)\n", args->fd,args->data,args->size);
    cons_tty(args->data);
    return EOK;
#if 0
    // jak to mniej wiecej powinno moim zdaniem wygladac:
    file_t *file = fd_get(args->fd, t->thr_proc->p_fd);
    if (f == NULL) {
        return EBADF;
    }
    uio_t u;
    iovec_t iov;
    iov.iov_base = args->data;
    iov.iov_len = args->size;
    u.iovs = &iov;
    u.iovcnt = 1;
    u.oper = UIO_WRITE;
    u.space = UIO_SYSTEM;
    u.owner = p;
    r->result = f_write(file, &u);
#endif
}

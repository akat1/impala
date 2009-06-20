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
 * $Id: TEMPLATE.c 405 2009-06-12 20:32:13Z takeshi $
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vm.h>
#include <fs/fifofs/fifofs.h>

typedef struct pipe_args pipe_args_t;
struct pipe_args {
    int *filedes;
};

int sc_pipe(thread_t *p, syscall_result_t *r, pipe_args_t *args);

int
sc_pipe(thread_t *t, syscall_result_t *r, pipe_args_t *args)
{
    int err = 0;
    proc_t *proc = t->thr_proc;
    if((err = vm_is_avail((vm_addr_t)args->filedes, sizeof(int[2]))))
        return err;

    vnode_t *p_read, *p_write;
    int fd1, fd2;
    if((err = fifo_create(&p_read, &p_write)))
        return err;
    if((err = f_alloc(proc, p_read, O_RDONLY, &fd1)))
        goto end_err;
    if((err = f_alloc(proc, p_write, O_WRONLY, &fd2)))
        goto end_err2;
    args->filedes[0] = fd1;
    args->filedes[1] = fd2;
    kprintf("PIPE created, %i, %i\n", fd1, fd2);
    return -EOK;

end_err2:
    kprintf("Pipe Err. 2\n");
    f_set(proc->p_fd, NULL, fd1);
    p_read = NULL;
end_err:
    kprintf("Pipe Err.\n");
    if(p_read)
        vrele(p_read);
    vrele(p_write);
    return err;
}


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
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vm.h>
#include <sys/vm/vm_space.h>


typedef struct thr_create_args thr_create_args_t;
struct thr_create_args {
    uintptr_t   entry;
    uintptr_t   stack_addr;
    uintptr_t   stack_size;
};

int sc_thr_create(thread_t *p, syscall_result_t *r, thr_create_args_t *args);

int
sc_thr_create(thread_t *t, syscall_result_t *r, thr_create_args_t *args)
{
    thread_t *ct = proc_create_thread(t->thr_proc, args->entry);
    if (ct == NULL) return -EINVAL;
    if (args->stack_size == 0 && args->stack_addr != 0) return -EINVAL;
//    if (args->stack_addr == 0) {
        if (args->stack_size == 0) args->stack_size = THREAD_STACK_SIZE;
        vm_space_create_stack(ct->vm_space, &ct->thr_stack, args->stack_size);
//    } else {
//        ct->thr_stack = (void*)args->stack_addr;
//    }
    ct->thr_stack_size = args->stack_size;
    TRACE_IN("new thread stack=%p+%p", ct->thr_stack, ct->thr_stack_size);
    thread_prepare(ct);
    sched_insert(ct);
    r->result = (uintptr_t)ct;
    return EOK;
}



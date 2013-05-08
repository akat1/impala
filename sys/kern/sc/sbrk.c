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

typedef struct sbrk_args sbrk_args_t;
struct sbrk_args {
    intptr_t    diff;
};

int sc_sbrk(thread_t *p, syscall_result_t *r, sbrk_args_t *args);

int
sc_sbrk(thread_t *t, syscall_result_t *r, sbrk_args_t *args)
{
    proc_t *p = t->thr_proc;
    if (args->diff > 0) {
        uintptr_t oldbrk = p->p_brk_addr;
        uintptr_t last_byte = p->p_brk_addr-1;
//        KASSERT(PAGE_ROUND(oldbrk) == p->vm_space->seg_data->end);
        if(PAGE_OF_ADDR(last_byte) != PAGE_OF_ADDR(last_byte+args->diff)) {
            //musimy alokować
            uintptr_t allocbeg;
            uintptr_t need = args->diff - (PAGE_ROUND(oldbrk)-oldbrk);
            vm_seg_alloc(t->vm_space->seg_data, need, &allocbeg);
        }
        r->result = p->p_brk_addr = oldbrk + args->diff;
//        kprintf("Sbrk: res = %x, diff = %i\n", r->result, args->diff);
    } else {
        TRACE_IN("PID: %u - decreasing heap not supported yet by kernel\n");
        proc_exit(p, 0);
        return -ENOTSUP;
    }
    return -EOK;
}



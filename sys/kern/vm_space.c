/* Impala Operating System
 *
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
#include <sys/vm.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/utils.h>


static int set_stack(vm_space_t *, vm_seg_t *, vm_seg_t *,
        vm_addr_t *, vm_size_t);

int
vm_space_create(vm_space_t *vs, int space)
{
    KASSERT(space == VM_SPACE_USER);
    vm_pmap_init(&vs->pmap);
    vs->space = space;
    vs->seg_text = vm_lpool_alloc(&vm_lpool_segments);
    vs->seg_data = vm_lpool_alloc(&vm_lpool_segments);
    vs->seg_stack = vm_lpool_alloc(&vm_lpool_segments);
    vm_seg_create(vs->seg_text, vs, VM_SPACE_UTEXT, 0, VM_SPACE_UTEXT_S,
        VM_PROT_RWX, VM_SEG_NORMAL);
    vm_seg_create(vs->seg_data, vs, VM_SPACE_UDATA, 0, VM_SPACE_UDATA_S,
        VM_PROT_RWX, VM_SEG_NORMAL);
    vm_seg_create(vs->seg_stack, vs, VM_SPACE_UDATA_E, 0, 0,
        VM_PROT_RWX, VM_SEG_EXPDOWN);
    return 0;
}

int
vm_space_clone(vm_space_t *dst, const vm_space_t *src)
{
    KASSERT(src->space == VM_SPACE_USER);
    vm_pmap_init(&dst->pmap);
    dst->space = src->space;
    vm_seg_clone(dst->seg_text, dst, src->seg_text);
    vm_seg_clone(dst->seg_data, dst, src->seg_data);
    vm_seg_clone(dst->seg_stack, dst, src->seg_stack);
    return 0;
}

int
vm_space_create_stack(vm_space_t *vs, void *_addr, vm_size_t s)
{
    vm_addr_t *addr = _addr;
    return set_stack(vs, vs->seg_stack, vs->seg_data, addr, s);
}


int
set_stack(vm_space_t *vs, vm_seg_t *STACK, vm_seg_t *DATA,
    vm_addr_t *res, vm_size_t s)
{
//     TRACE_IN("vs=%p STACK=%p DATA=%p res=%p size=%p",vs, STACK, DATA,
//         res, s);
    KASSERT(s > 0);
    s = PAGE_ROUND(s);

    vm_size_t stackspace = STACK->end - (DATA->base + DATA->limit) + PAGE_SIZE;
    if (stackspace < s) {
        if ((DATA->limit - DATA->size) < s) return -1;
        DATA->limit -= (s-stackspace);
    }

    STACK->limit += s;
//     TRACE_IN("here");
    return vm_seg_alloc(STACK, s, res);
}

void
vm_space_print(vm_space_t *vs)
{
    extern void ssleep(int);
    DEBUGF("vm_space_t %p", vs);
    DEBUGF("    TEXT    %p-%p %p (%ukB)", vs->seg_text->base,
        vs->seg_text->end, vs->seg_text->limit, vs->seg_text->size/1024);
    DEBUGF("    DATA    %p-%p %p (%ukB)", vs->seg_data->base, vs->seg_data->end,
        vs->seg_data->limit, vs->seg_data->size/1024);
    DEBUGF("    STACK   %p-%p %p (%ukB)", vs->seg_stack->base, vs->seg_stack->end,
        vs->seg_stack->limit, vs->seg_stack->size/1024);
}

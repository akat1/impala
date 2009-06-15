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

#ifndef __SYS_VM_VM_SPACE_H
#define __SYS_VM_VM_SPACE_H
#ifdef __KERNEL

struct vm_space {
    vm_pmap_t       pmap;
    vm_seg_t       *seg_text;
    vm_seg_t       *seg_data;
    vm_seg_t       *seg_stack;
    list_node_t     L_spaces;
    int             space;
    mutex_t        *mtx;
};

enum {
    VM_SPACE_SYSTEM,
    VM_SPACE_USER
};

int vm_space_create(vm_space_t *, int space);
int vm_space_clone(vm_space_t *space, const vm_space_t *src);
void vm_space_destroy(vm_space_t *space);
void vm_space_print(vm_space_t *vs);
int vm_space_create_stack(vm_space_t *, void *addr, vm_size_t s);
int vm_space_destroy_stack(vm_space_t *, vm_addr_t, vm_size_t);
int vm_space_is_avail(vm_space_t *vs, vm_addr_t addr, vm_size_t s);


#endif
#endif

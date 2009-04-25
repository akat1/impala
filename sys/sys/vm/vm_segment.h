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

#ifndef __SYS_VM_VM_SEG_H
#define __SYS_VM_VM_SEG_H

struct vm_segment {
    vm_space_t     *space;
    vm_addr_t       base;
    vm_addr_t       end;
    size_t          size;
    size_t          limit;
    int             flags;
    list_t          regions;
};

struct vm_region {
    vm_addr_t       begin;
    vm_addr_t       end;
    size_t          size;
    list_node_t     L_regions;
    int             flags;
};


enum VM_SEGMENT_FLAGS {
    VM_SEG_EXPAND_DOWN      = 1 << 0
};

enum VM_REGION_FLAGS {
    VM_REGION_FREE          = 0,
    VM_REGION_USED          = 1 << 0
};

void vm_segment_create(vm_segment_t *vs, vm_space_t *s, vm_addr_t base,
        size_t len, size_t limit);
//void vm_segment_expand(vm_segment_t *vs, size_t size);
vm_page_t * vm_segment_expand(vm_segment_t *vms, vm_addr_t *_va);

vm_addr_t vm_segment_alloc(vm_segment_t *vs, size_t size);
void vm_segment_free(vm_segment_t *vs, vm_addr_t size, size_t length);


#endif

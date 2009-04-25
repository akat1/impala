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

#ifndef __SYS_VM_VM_LPOOL_H
#define __SYS_VM_VM_LPOOL_H

/// Ma�y zarz�dzacz pami�ci� do ma�ych element�w, spinanych w liste.
struct vm_lpool {
    list_t  empty_pools;
    list_t  part_pools;
    list_t  full_pools;
    int     elems_per_page;
    size_t  elem_size;
    int     offset;
    int     flags;
};

enum {
    VM_LPOOL_NORMAL     = 0,
    VM_LPOOL_PREALLOC   = 1 << 0
};

void vm_lpool_create(vm_lpool_t *vp, int off, size_t size, int flags);
void vm_lpool_create_(vm_lpool_t *vp, int off, size_t size, int flags, void *page);
void vm_lpool_insert_empty(vm_lpool_t *vm, void *page);
void vm_lpool_destroy(vm_lpool_t *vp);

void *vm_lpool_alloc(vm_lpool_t *vp);
void vm_lpool_free(vm_lpool_t *vp, void *x);




#endif


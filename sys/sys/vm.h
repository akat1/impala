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


#ifndef __SYS_VM_H
#define __SYS_VM_H
#ifdef __KERNEL

#include <sys/vm/vm_types.h>
#include <machine/memory.h>
#include <sys/vm/vm_pmap.h>
#include <sys/vm/vm_segment.h>
#include <sys/vm/vm_space.h>

#define PAGE_MASK (PAGE_SIZE-1)
#define YPAGE_ROUND(x) (((x) + PAGE_MASK)/PAGE_SIZE)
#define PAGE_ROUND(x) (((x) + PAGE_MASK) & ~PAGE_MASK)

extern list_t vm_free_pages;
extern vm_space_t vm_kspace;

void vm_init(void);
void vm_lock(void);
void vm_unlock(void);
bool vm_trylock(void);

vm_page_t *vm_alloc_page(void);
void vm_free_page(vm_page_t *p);

vm_paddr_t vm_space_phys(const vm_space_t *vms, vm_addr_t addr);
void vm_space_switch(const vm_space_t *sp);
vm_page_t *vm_kernel_alloc_page(void);
vm_addr_t vm_ptov(vm_paddr_t v);
vm_paddr_t vm_vtop(vm_addr_t p);

int vm_segmap(vm_seg_t *seg, vm_addr_t addr, vm_size_t s, void *res);
int vm_physmap(vm_addr_t paddr, vm_size_t s, void *res);
int vm_remap(vm_addr_t vaddr, vm_size_t s, void *res);
void vm_unmap(vm_addr_t addr, vm_size_t size);

#define vm_is_avail(vms, addr) vm_space_is_avail(vms, (vm_addr_t) addr)

#endif
#endif

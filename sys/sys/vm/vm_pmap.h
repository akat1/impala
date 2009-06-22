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


#ifndef __SYS_VM_VM_PMAP_H
#define __SYS_VM_VM_PMAP_H

enum {
    VM_PROT_READ = 1<<0,
    VM_PROT_WRITE = 1<<1,
    VM_PROT_EXEC = 1<<2,
    VM_PROT_USER = 1<<3,
    VM_PROT_SYSTEM = 1<<4,
    VM_PROT_RWX = VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXEC,
    VM_PROT_RX = VM_PROT_READ | VM_PROT_EXEC
};

enum {
    VM_GHOST_ONDEMAND = 1,
};

bool vm_pmap_init(vm_pmap_t *);
bool vm_pmap_insert(vm_pmap_t *, vm_page_t *, vm_addr_t, vm_prot_t);
bool vm_pmap_insert_ghost(vm_pmap_t *, vm_paddr_t, vm_addr_t, int ghost);
bool vm_pmap_insert_(vm_pmap_t *, vm_paddr_t , vm_addr_t, vm_prot_t);
bool vm_pmap_fill(vm_pmap_t *, vm_addr_t , vm_size_t, vm_prot_t);
void vm_pmap_fillprot(vm_pmap_t *, vm_addr_t , vm_size_t, vm_prot_t);
void vm_pmap_physmap(vm_pmap_t *, vm_addr_t , vm_paddr_t, vm_size_t, vm_prot_t);
void vm_pmap_map(vm_pmap_t *dst, vm_addr_t dst_addr,  const vm_pmap_t *src,
    vm_addr_t src_addr, vm_size_t size);
bool vm_pmap_erase(vm_pmap_t *, vm_addr_t, vm_size_t);
bool vm_pmap_remove(vm_pmap_t *, vm_addr_t);
vm_paddr_t vm_pmap_phys(const vm_pmap_t *, vm_addr_t );
bool vm_pmap_is_avail(const vm_pmap_t *, vm_addr_t);
void vm_pmap_switch(const vm_pmap_t *);
void vm_pmap_clone(vm_pmap_t *, const vm_pmap_t *);

#endif

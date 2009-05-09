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

bool vm_pmap_init(vm_pmap_t *vpm);
bool vm_pmap_insert(vm_pmap_t *vpm, vm_page_t *p, vm_addr_t va);
bool vm_pmap_insert_(vm_pmap_t *vpm, vm_paddr_t pa, vm_addr_t va);
bool vm_pmap_remove(vm_pmap_t *vpm, vm_addr_t addr);
vm_paddr_t vm_pmap_phys(const vm_pmap_t *vpm, vm_addr_t va);
bool vm_pmap_is_avail(const vm_pmap_t *vms, vm_addr_t addr);
void vm_pmap_switch(const vm_pmap_t *pmap);

void vm_kpmap_init(void);
void vm_kpmap_insert(vm_page_t *p, vm_addr_t va);
void vm_kpmap_insert_(vm_paddr_t pa, vm_addr_t va);
void vm_kpmap_is_avail(vm_addr_t addr);
    
#endif

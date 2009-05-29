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
#include <sys/vm/vm_pmap.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/thread.h>
#include <sys/utils.h>

/// Globalny zamek pamiêci wirtualnej.
static spinlock_t vm_sp;
vm_lpool_t vm_unused_regions;
list_t vm_spaces;
vm_space_t vm_kspace;

/// Lista wolnych stron.
list_t vm_free_pages;
vm_lpool_t vm_lpool_segments;

/// Inicjalizuje system pamiêci wirtualnej.
void
vm_init()
{
    spinlock_init(&vm_sp);
    list_create(&vm_spaces, offsetof(vm_space_t, L_spaces), FALSE);
    list_insert_head(&vm_spaces, &vm_kspace);
    vm_lpool_create(&vm_lpool_segments, offsetof(vm_seg_t, L_segments),
        sizeof(vm_seg_t), VM_LPOOL_NORMAL);
}


/// Zamyka zamek systemu pamiêci wirtualnej.
void
vm_lock()
{
    spinlock_lock(&vm_sp);
}

/// Zwalnia zamek systemu pamiêci wirtualnej.
void
vm_unlock()
{
    spinlock_unlock(&vm_sp);
}

/**
 * Przydziela stronê pamiêci.
 * @return Zwracan adres uchwytu strony lub NULL gdy nie ma wolnych.
 */
vm_page_t *
vm_alloc_page()
{
    vm_page_t *p = list_extract_first(&vm_free_pages);
    if (p) vm_physmem_free--;
    return p;
}

/**
 * Zwalnia stronê.
 * @param p Uchwyt strony.
 */
void
vm_free_page(vm_page_t *p)
{
    p->flags = 0;
    list_insert_head(&vm_free_pages, p);
    vm_physmem_free++;
}

vm_paddr_t
vm_space_phys(const vm_space_t *vms, vm_addr_t addr)
{
    return vm_pmap_phys(&vms->pmap, addr);
}


void
vm_space_switch(const vm_space_t *vms)
{
    vm_pmap_switch(&vms->pmap);
}


vm_paddr_t
vm_vtop(vm_addr_t va)
{
    return vm_pmap_phys(&vm_kspace.pmap, va);
}


int
vm_segmap(vm_seg_t *seg, vm_addr_t addr, vm_size_t s, void *res)
{
    return vm_seg_map(vm_kspace.seg_data, seg, addr, s, res);
}

int
vm_remap(vm_addr_t vaddr, vm_size_t s, void *res)
{
    s = PAGE_ROUND(s);
    vm_addr_t *vres = res;
    if (vm_seg_reserve(vm_kspace.seg_data, s, res))
        return -1;
    vm_pmap_map(&vm_kspace.pmap, *vres, &curthread->vm_space->pmap,
        vaddr, s);
    return 0;
}

int
vm_physmap(vm_paddr_t paddr, vm_size_t s, void *res)
{
    s = PAGE_ROUND(s);
    vm_addr_t *vres = res;
    if (vm_seg_reserve(vm_kspace.seg_data, s, res))
        return -1;
    vm_pmap_physmap(&vm_kspace.pmap, *vres, paddr, s, vm_kspace.seg_data->prot);
    return 0;
}

void
vm_unmap(vm_addr_t addr, vm_size_t size)
{
    vm_seg_free(vm_kspace.seg_data, addr, size);
}

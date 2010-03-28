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
#include <sys/errno.h>

vm_lpool_t vm_unused_regions;
list_t vm_spaces;
vm_space_t vm_kspace;

/// Lista wolnych stron.
list_t vm_free_pages;
vm_lpool_t vm_lpool_segments;

static mutex_t vm_mtx;
static bool sync = FALSE;
#define VM_LOCK() if (sync) mutex_lock(&vm_mtx, __FILE__, __func__, __LINE__, "VM" )
#define VM_UNLOCK() if (sync) mutex_unlock(&vm_mtx)

/// Inicjalizuje system pamięci wirtualnej.
void
vm_init()
{
    mutex_init(&vm_mtx, MUTEX_NORMAL);
    list_create(&vm_spaces, offsetof(vm_space_t, L_spaces), FALSE);
    list_insert_head(&vm_spaces, &vm_kspace);
    vm_lpool_create(&vm_lpool_segments, offsetof(vm_seg_t, L_segments),
        sizeof(vm_seg_t), VM_LPOOL_NORMAL);
    sync = TRUE;
}


/// Zamyka zamek systemu pamięci wirtualnej.
void
vm_lock()
{
    VM_LOCK();
}

/// Zwalnia zamek systemu pamięci wirtualnej.
void
vm_unlock()
{
    VM_UNLOCK();
}

/**
 * Przydziela stronę pamięci.
 * @return Zwracan adres uchwytu strony lub NULL gdy nie ma wolnych.
 */
vm_page_t *
vm_alloc_page()
{
    VM_LOCK();
    vm_page_t *p = list_extract_first(&vm_free_pages);
    if (p) vm_physmem_free--;
    VM_UNLOCK();
    return p;
}

/**
 * Zwalnia stronę.
 * @param p Uchwyt strony.
 */
void
vm_free_page(vm_page_t *p)
{
    VM_LOCK();
    p->flags = 0;
    list_insert_head(&vm_free_pages, p);
    vm_physmem_free++;
    VM_UNLOCK();
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
    VM_LOCK();
    s = PAGE_ROUND(s);
    vm_addr_t *vres = res;
    if (vm_seg_reserve(vm_kspace.seg_data, s, res)) {
        VM_UNLOCK();
        return -1;
    }
    vm_pmap_map(&vm_kspace.pmap, *vres, &curthread->vm_space->pmap,
        vaddr, s);
    VM_UNLOCK();
    return 0;
}

int
vm_physmap(vm_paddr_t paddr, vm_size_t s, void *res)
{
    VM_LOCK();
    s = PAGE_ROUND(s);
    vm_addr_t *vres = res;
    if (vm_seg_reserve(vm_kspace.seg_data, s, res)) {
        VM_UNLOCK();
        return -1;
    }
    vm_pmap_physmap(&vm_kspace.pmap, *vres, paddr, s, vm_kspace.seg_data->prot);
    VM_UNLOCK();
    return 0;
}

void
vm_unmap(vm_addr_t addr, vm_size_t size)
{
    VM_LOCK();
    vm_seg_free(vm_kspace.seg_data, addr, size);
    VM_UNLOCK();
}

/**
 *  Weryfikuje, czy napis wskazywany przez str jest poprawnym napisem
 *  mieszczącym się razem z kończącym zerem w maxlen bajtach.
 *  Poprawność napisu oznacza, że wykorzystywane przez niego strony pamięci
 *  są zaalokowane i należą do pamięci użytkownika.
 *
 *  UWAGA: maxlen nie może pochodzić od użytkownika. Nie może przekroczyć 1GB.
 */

int
vm_validate_string(const char *str, const size_t maxlen)
{
    vm_pmap_t *pmap = &curthread->vm_space->pmap;
    int old_page = PAGE_NUM(str);
    if( (vm_addr_t)str >= VM_SPACE_KERNEL)
        return -EFAULT;
    
    for(size_t i=0; i<maxlen; i++) {
        if(old_page != PAGE_NUM(str+i)) {
            old_page = PAGE_NUM(str+i);
            if(!vm_pmap_is_avail(pmap, (vm_addr_t)&str[i])) return -EFAULT;
        }
        if(str[i] == 0)
            return ((vm_addr_t)&str[i] >= VM_SPACE_KERNEL)? -EFAULT : i;
    }
    return -ENAMETOOLONG;
}

int
vm_is_avail(vm_addr_t addr, vm_size_t s)
{
    vm_pmap_t *pmap = &curthread->vm_space->pmap;
    s = PAGE_ROUND(s);
//    if (addr + s >= vm_kspace.seg_text->base) return -EFAULT;
    for (s += addr; addr < s; addr += PAGE_SIZE) {
        if (!vm_pmap_is_avail(pmap, addr) || addr >= VM_SPACE_KERNEL)
            return -EFAULT;
    }
    return 0;
}

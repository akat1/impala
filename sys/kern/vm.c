/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/list.h>
#include <sys/vm.h>
#include <sys/vm/vm_pmap.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/thread.h>

/// Globalny zamek pamiêci wirtualnej.
static spinlock_t vm_sp;
vm_lpool_t vm_unused_regions;

/// Lista wolnych stron.
list_t vm_free_pages;


/// Inicjalizuje system pamiêci wirtualnej.
void
vm_init()
{
    spinlock_init(&vm_sp);
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


vm_page_t *
vm_segment_expand(vm_segment_t *vms, vm_addr_t *_va)
{
    if (vms->base + vms->size + PAGE_SIZE <= vms->limit) {
        vm_addr_t va;
        vm_page_t *p = vm_alloc_page();
        va = vms->base + vms->size;
        vm_pmap_insert(&vms->space->pmap, p, va); 
        p->flags &= ~PAGE_FREE;
        vms->size += PAGE_SIZE;
        if (_va) *_va = va;
        return p;
    } else {
        return NULL;
    }
}

vm_paddr_t
vm_space_phys(const vm_space_t *vms, vm_addr_t addr)
{
    return vm_pmap_phys(&vms->pmap, addr);
}

bool
vm_space_is_avail(const vm_space_t *vms, vm_addr_t addr)
{
    return vm_pmap_is_avail(&vms->pmap, addr);
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


vm_page_t *
vm_kernel_alloc_page()
{
    vm_addr_t x;
    vm_page_t *p = vm_segment_expand(&vm_kspace.seg_data, &x);
    p->kvirt_addr = x;
    return p;
}


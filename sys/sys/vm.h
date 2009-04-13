/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_VM
#define __SYS_VM

#include <sys/vm/vm_types.h>
#include <machine/memory.h>
#include <sys/vm/vm_pmap.h>
#include <sys/vm/vm_segment.h>
#include <sys/vm/vm_space.h>


extern list_t vm_free_pages;
extern list_t vm_unused_regions;
extern vm_space_t vm_kspace;

void vm_init(void);
void vm_lock(void);
void vm_unlock(void);
bool vm_trylock(void);

vm_page_t *vm_alloc_page(void);
void vm_free_page(vm_page_t *p);

vm_paddr_t vm_space_phys(const vm_space_t *vms, vm_addr_t addr);
bool vm_space_is_avail(const vm_space_t *vms, vm_addr_t addr);
void vm_space_switch(const vm_space_t *sp);

vm_page_t *vm_kernel_alloc_page(void);

vm_addr_t vm_ptov(vm_paddr_t v);
vm_paddr_t vm_vtop(vm_addr_t p);



#endif

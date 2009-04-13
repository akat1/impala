#ifndef __SYS_VM_VM_PMAP_H
#define __SYS_VM_VM_PMAP_H

bool vm_pmap_init(vm_pmap_t *vpm);
bool vm_pmap_insert(vm_pmap_t *vpm, vm_page_t *p, vm_addr_t va);
bool vm_pmap_insert_(vm_pmap_t *vpm, vm_paddr_t pa, vm_addr_t va);
vm_page_t *vm_pmap_remove(vm_pmap_t *vpm, vm_addr_t addr);
vm_paddr_t vm_pmap_phys(const vm_pmap_t *vpm, vm_addr_t va);
bool vm_pmap_is_avail(const vm_pmap_t *vms, vm_addr_t addr);
void vm_pmap_switch(const vm_pmap_t *pmap);

void vm_kpmap_init(void);
void vm_kpmap_insert(vm_page_t *p, vm_addr_t va);
void vm_kpmap_insert_(vm_paddr_t pa, vm_addr_t va);
void vm_kpmap_is_avail(vm_addr_t addr);
    
#endif

#ifndef __SYS_VM_VM_SEG_H
#define __SYS_VM_VM_SEG_H

struct vm_segment {
    vm_space_t     *space;
    vm_addr_t       base;
    size_t          size;
    size_t          limit;
    int             flags;
    list_t          regions;
};

struct vm_region {
    vm_addr_t       begin;
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

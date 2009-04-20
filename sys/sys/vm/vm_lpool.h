/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_VM_VM_LPOOL_H
#define __SYS_VM_VM_LPOOL_H

/// Ma³y zarz±dzacz pamiêci± do ma³ych elementów, spinanych w liste.
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


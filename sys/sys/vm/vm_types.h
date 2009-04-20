/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_VM_VM_TYPES_H
#define __SYS_VM_VM_TYPES_H

typedef uintptr_t vm_addr_t;
typedef uintptr_t vm_paddr_t;
typedef uintptr_t vm_size_t;

typedef struct vm_page vm_page_t;
typedef struct vm_pmap vm_pmap_t;
typedef struct vm_space vm_space_t;
typedef struct vm_segment vm_segment_t;
typedef struct vm_region vm_region_t;
typedef struct vm_ptable vm_ptable_t;
typedef struct vm_lpool vm_lpool_t;

#endif

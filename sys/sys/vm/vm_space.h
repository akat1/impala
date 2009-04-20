/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_VM_VM_SPACE_H
#define __SYS_VM_VM_SPACE_H

struct vm_space {
    vm_pmap_t      pmap;
    vm_segment_t   seg_code;
    vm_segment_t   seg_data;
    vm_segment_t   seg_stack;
};


#endif

#ifndef __SYS_VM_VM_TRAP_H
#define __SYS_VM_VM_TRAP_H


typedef struct vm_trap_frame vm_trap_frame_t;
struct vm_trap_frame {
    vm_addr_t   fault_addr;
    vm_addr_t   preempted_addr;
    int         reason;
    int         operation;
    bool        in_kernel;
};


enum VM_PFAULT_REASON {
    VM_PFAULT_NO_PRESENT,
    VM_PFAULT_NO_PERMISSION,
    VM_PFAULT_OTHER
};

enum VM_OPERATION {
    VM_READ,
    VM_WRITE
};

void vm_trap_pfault(vm_trap_frame_t *f);


#endif

/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/vm.h>
#include <sys/vm/vm_trap.h>
#include <sys/thread.h>
#include <sys/kprintf.h>
#include <sys/utils.h>


static const char *__pfault_reason_to_str(int p);
static const char *__page_operation_to_str(int p);
static void vm_user_pfault(vm_trap_frame_t *frame);

void
vm_trap_pfault(vm_trap_frame_t *frame)
{
    kprintf("page fault: %p\n", frame->fault_addr);
    kprintf(" reason:    %s\n", __pfault_reason_to_str(frame->reason));
    kprintf(" operation: %s\n", __page_operation_to_str(frame->operation));
    kprintf(" caused by: %s at %p\n",
        (frame->in_kernel)? "kernel code" : "user code",
        frame->preempted_addr);
    kprintf(" curthread: %p 0x%x\n", curthread, curthread->thr_flags);

    if (frame->in_kernel) {
        panic("kernel fatal error");
    } else {
        vm_user_pfault(frame);
    }
}

void
vm_user_pfault(vm_trap_frame_t *frame)
{
    panic("page faults for user code not supported yet");
}



static const char *
__pfault_reason_to_str(int p)
{
    const char *msg;
    switch (p) {
        case VM_PFAULT_NO_PRESENT:
            msg = "page not present";
            break;

        case VM_PFAULT_NO_PERMISSION:
            msg = "access violation";
            break;

        case VM_PFAULT_OTHER:
            msg = "<unknown reason>";
            break;

        default:
            msg = "<bad-value>";
            break;
    }
    return msg;
}

static const char *
__page_operation_to_str(int p)
{
    const char *msg;
    switch (p) {
        case VM_READ:
            msg = "read";
            break;

        case VM_WRITE:
            msg = "write";
            break;

        default:
            msg = "<bad-value>";
            break;
    }
    return msg;
}


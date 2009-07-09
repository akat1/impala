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
#include <sys/vm/vm_trap.h>
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/proc.h>
#include <sys/wait.h>

// #include <machine/cpu.h>

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
    kprintf(" image:     %s\n", curthread->thr_proc->p_cmd);
//     kprintf(" vm_space:  %p,%p,%p\n", curthread->vm_space,
//         curthread->vm_space->pmap.physdir, cpu_get_cr3());

    if (frame->in_kernel) {
        panic("kernel fatal error");
    } else {
        vm_user_pfault(frame);
    }
}

void
vm_user_pfault(vm_trap_frame_t *frame)
{
    kprintf("Process PID=%u: access violation (killed)\n",
        curthread->thr_proc->p_pid);
    signal_send(curthread->thr_proc, SIGSEGV);
    signal_handle(curthread);
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


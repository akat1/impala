/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/kprintf.h>
#include <sys/thread.h>
#include <sys/syscall.h>
#include <sys/utils.h>
#include <sys/vm.h>
#include <sys/vm/vm_trap.h>
#include <machine/interrupt.h>
#include <machine/i8259a.h>
#include <machine/memory.h>
#include <machine/cpu.h>


void ISR_irq(interrupt_frame f);
void ISR_syscall(interrupt_frame frame);
void TRAP_unhandled(interrupt_frame fr);
void TRAP_gfault(interrupt_frame fr);
void TRAP_pfault(interrupt_frame fr);
static void print_frame(const char *s, interrupt_frame *f);

irq_handler_f *irq_handlers[MAX_IRQ];

void
irq_install_handler(int irq, irq_handler_f *f)
{
    if (irq <= MAX_IRQ) {
         irq_handlers[irq] = f;
         i8259a_irq_enable(irq);
    }
}

void
irq_free_handler(int irq)
{
    if (irq <= MAX_IRQ) {
         i8259a_irq_disable(irq);
         irq_handlers[irq] = NULL;
    }
}

void
irq_done()
{
    irq_enable();
    i8259a_send_eoi();
}

void
ISR_irq(interrupt_frame frame)
{
    bool eoi = FALSE;
    if (frame.f_n <= MAX_IRQ) {
        eoi = irq_handlers[frame.f_n]();
    }
    if (!eoi) i8259a_send_eoi();
}

void
ISR_syscall(interrupt_frame frame)
{
    va_list ap;
    ap = (va_list) (frame.f_esp+4);
    curthread->thr_flags |= THREAD_SYSCALL;
    syscall(curthread, frame.f_eax, ap);
    curthread->thr_flags ^= THREAD_SYSCALL;
}

void
TRAP_unhandled(interrupt_frame f)
{
    print_frame("unhandled exception", &f);
    __asm__("hlt");
}

void
TRAP_gfault(interrupt_frame f)
{
    //panic("General protection fault\n");
    print_frame("general protection fault", &f);
    __asm__("hlt");
}

void
TRAP_pfault(interrupt_frame f)
{
    vm_trap_frame_t vtf;
    vm_disable_paging();
    vtf.fault_addr = cpu_get_cr2(); 
    vtf.reason = (f.f_errno & PFE_PRESENT)?
             VM_PFAULT_NO_PERMISSION
            : VM_PFAULT_NO_PRESENT;
    vtf.in_kernel = !(f.f_errno & PFE_US);
    vtf.operation = (f.f_errno & PFE_WR)? VM_WRITE : VM_READ;
    vtf.preempted_addr = f.f_eip;
    vm_trap_pfault(&vtf);
}

void
print_frame(const char *name, interrupt_frame *f)
{
    kprintf("\n%s\n", name);
    kprintf("   #     = %u ", f->f_n);
    kprintf("   errno = %u\n", f->f_errno);
    kprintf("   preempted at %p\n", f->f_eip);
/* malo potrzebne
    kprintf("   %%eax = %p ", f->f_eax);
    kprintf("   %%ebx = %p\n", f->f_ebx);
    kprintf("   %%ecx = %p ", f->f_ecx);
    kprintf("   %%edx = %p\n", f->f_edx);
*/
    kprintf("   %%cs  = %p ", f->f_cs);
    kprintf("   %%ds  = %p\n", f->f_ds);
    kprintf("   %%es  = %p ", f->f_es);
    kprintf("   %%fs  = %p\n", f->f_fs);
    kprintf("   %%gs  = %p ", f->f_fs);
    kprintf("   %%ss  = %p\n", f->f_ss);
    kprintf("   %%esp = %p ", f->f_esp);
    kprintf("   %%ebp = %p\n", f->f_ebp);
}


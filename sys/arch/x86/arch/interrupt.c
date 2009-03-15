#include <sys/types.h>
#include <machine/cpu.h>
#include <machine/interrupt.h>
#include <machine/i8259a.h>
#include <sys/kprintf.h>
#include <sys/thread.h>
#include <sys/syscall.h>
#include <sys/utils.h>

void ISR_irq(interrupt_frame f);
void ISR_syscall(interrupt_frame frame);
void TRAP_unhandled(void);
void TRAP_gfault(void);

irq_handler_f *irq_handlers[MAX_IRQ];

void irq_install_handler(int irq, irq_handler_f *f)
{
    if (irq <= MAX_IRQ) {
         irq_handlers[irq] = f;
         i8259a_irq_enable(irq);
    }
}


void
irq_done()
{
    sti();
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
TRAP_unhandled()
{
}

void
TRAP_gfault()
{
    panic("General protection fault\n");
}

#include <sys/utils.h>
#include <sys/kprintf.h>
#include <machine/interrupt.h>

/**
 * Funkcja wywo³ywana w sytuacjach awaryjnych.
 * Zatrzymuje system, wy¶wietlaj±c podany komunikat.
 */

void
panic(const char *msg, ...)
{
    irq_disable();
    va_list ap;
    VA_START(ap, msg);
    kprintf("\n\nkernel panic: ");
    vkprintf(msg, ap);
    kprintf("\n\n");
    VA_END(ap);
    while(1);
}

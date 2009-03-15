#include <machine/cpu.h>
#include <sys/utils.h>
#include <sys/kprintf.h>


/**
 * Funkcja wywoływana w sytuacjach awaryjnych.
 * Zatrzymuje system, wyświetlając podany komunikat.
 */

void
panic(const char *msg, ...)
{
    cli();
    va_list ap;
    VA_START(ap, msg);
    kprintf("\n\nkernel panic: ");
    vkprintf(msg, ap);
    kprintf("\n\n");
    VA_END(ap);
    while(1);
}

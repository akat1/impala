/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/utils.h>
#include <sys/kprintf.h>
#include <machine/interrupt.h>

/**
 * Funkcja wywoływana w sytuacjach awaryjnych.
 * Zatrzymuje system, wyświetlając podany komunikat.
 */

void
panic(const char *msg, ...)
{
    irq_disable();
    va_list ap;
    VA_START(ap, msg);
    kprintf("\npanic: ");
    vkprintf(msg, ap);
    kprintf("\n");
    VA_END(ap);
    while(1);
}

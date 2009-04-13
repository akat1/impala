/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/clock.h>
#include <sys/kprintf.h>
#include <sys/utils.h>
#include <machine/interrupt.h>
#include <machine/i8254.h>
#include <machine/i8259a.h>
#include <machine/io.h>

static bool i8254_irq0(void);


void
i8254_init()
{
    i8254_set_freq(HZ);
    irq_install_handler(IRQ0, i8254_irq0);
}

bool
i8254_irq0()
{
    clock_hardtick();
    irq_done();
    clock_softtick();
    return TRUE;
}

/**
 * Ustawia czêstotliwo¶æ z jak± PIT generuje IRQ 0
 */

void
i8254_set_freq(uint hz)
{
    KASSERT(hz >= 19);  // wymagane, aby wynik mie¶ci³ siê w 2 bajtach
    
    uint res=PIT_MAX_FREQ/hz;
    
    io_out8(PIT_MODE, 0x34); // licznik zero w trybie 2,
                             // przesy³ane oba bajty kodowanej binarnie liczby
    io_out8(PIT_CHAN0, res&0xff);
    io_out8(PIT_CHAN0, (res>>8)&0xff);
}


/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/clock.h>
#include <sys/kprintf.h>
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

void
i8254_set_freq(uint hz)
{
    kprintf("i8254 interrupt timer: %uhz\n", hz);
    // brakuje chyba wys³ania komendy resetuj±cej chipset
    uint res=PIT_MAX_FREQ/hz;
    //TODO jakies ,,zaawansowane obliczenia'' co do HZ
    //io_out8(0x34, PIT_MODE);
    io_out8(PIT_CHAN0, res&0xff);
    io_out8(PIT_CHAN0, (res>>8)&0xff);
}


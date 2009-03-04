#include <sys/types.h>
#include <sys/clock.h>
#include <sys/list.h>
#include <sys/sched.h>
#include <sys/thread.h>
#include <machine/interrupt.h>
#include <machine/atomic.h>


static spinlock_t soft_guard = { SPINLOCK_LOCK };

volatile uint clock_ticks;

void
clock_init()
{
     spinlock_init(&soft_guard);
}

/* ,,twarde tykniêcie''
 * Procedura uruchamiana wewn±trz obs³ugi przerwania zegara, nie mo¿e
 * opó¼niæ kolejnego tykniêcia!.
 */
void
clock_hardtick()
{
    clock_ticks++;
}

/* ,,miêkkie tykniêcie''
 * Procedura uruchamiana na zewn±trz obs³ugi przerwania zegara. Czas procesora
 * zajêty przez ni± opó¼nia jej kolejne wywo³anie, a nie przerwanie.
 */

#include <sys/kprintf.h>

void
clock_softtick()
{
    if ( spinlock_trylock(&soft_guard) ) {
        spinlock_unlock(&soft_guard);
        sched_action();
    }
}


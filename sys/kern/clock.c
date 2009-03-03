#include <sys/types.h>
#include <sys/clock.h>
#include <sys/list.h>
#include <sys/sched.h>
#include <sys/thread.h>
#include <machine/interrupt.h>
#include <machine/atomic.h>

enum {
    TICK_UNLOCKED,
    TICK_LOCKED,
    TICK_DISABLED
};

static uint entered = TICK_DISABLED;
volatile uint clock_ticks;

void
clock_init()
{
    atomic_change32(&entered, TICK_UNLOCKED);
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

void
clock_softtick()
{
//     if (atomic_change32(&entered, TICK_LOCKED) != TICK_UNLOCKED) return;
//     atomic_change32(&entered, TICK_UNLOCKED);
    sched_action();

}


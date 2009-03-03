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

/* ,,twarde tykni�cie''
 * Procedura uruchamiana wewn�trz obs�ugi przerwania zegara, nie mo�e
 * op�ni� kolejnego tykni�cia!.
 */
void
clock_hardtick()
{
    clock_ticks++;
}

/* ,,mi�kkie tykni�cie''
 * Procedura uruchamiana na zewn�trz obs�ugi przerwania zegara. Czas procesora
 * zaj�ty przez ni� op�nia jej kolejne wywo�anie, a nie przerwanie.
 */

void
clock_softtick()
{
//     if (atomic_change32(&entered, TICK_LOCKED) != TICK_UNLOCKED) return;
//     atomic_change32(&entered, TICK_UNLOCKED);
    sched_action();

}


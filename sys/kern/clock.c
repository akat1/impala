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

#include <sys/kprintf.h>

void
clock_softtick()
{
    if ( spinlock_trylock(&soft_guard) ) {
        spinlock_unlock(&soft_guard);
        sched_action();
    }
}


#include <sys/types.h>
#include <sys/clock.h>
#include <sys/list.h>
#include <sys/sched.h>
#include <sys/thread.h>
#include <machine/interrupt.h>
#include <machine/atomic.h>

/// Wiruj�cy zamek zabezpieczaj�cy mi�kkie tykni�cie.
static spinlock_t soft_guard = { SPINLOCK_LOCK };

/// Licznik tykni��.
volatile uint clock_ticks;

/**
 * Inicjalizuje obs�ug� tykni�� zegara systemowego.
 */

void
clock_init()
{
     spinlock_init(&soft_guard);
}


/**
 * @brief Twarde tykni�cie zegara.
 *
 *
 * Procedura uruchamiana wewn�trz obs�ugi przerwania zegara, nie mo�e
 * op�ni� kolejnego tykni�cia.
 */

void
clock_hardtick()
{
    clock_ticks++;
}

/**
 * @brief Mi�kkie tykni�cie.
 *
 * Procedura uruchamiana nazewn�trz obs�ugi przerwania zegara. Czas procesora
 * zaj�ty przez ni� op�nia kolejne jej wywo�anie, nie przerwania.
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


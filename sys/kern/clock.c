#include <sys/types.h>
#include <sys/clock.h>
#include <sys/list.h>
#include <sys/sched.h>
#include <sys/thread.h>
#include <machine/interrupt.h>
#include <machine/atomic.h>

/// Wiruj±cy zamek zabezpieczaj±cy miêkkie tykniêcie.
static spinlock_t soft_guard = { SPINLOCK_LOCK };

/// Licznik tykniêæ.
volatile uint clock_ticks;

/**
 * Inicjalizuje obs³ugê tykniêæ zegara systemowego.
 */

void
clock_init()
{
     spinlock_init(&soft_guard);
}


/**
 * @brief Twarde tykniêcie zegara.
 *
 *
 * Procedura uruchamiana wewn±trz obs³ugi przerwania zegara, nie mo¿e
 * opó¼niæ kolejnego tykniêcia.
 */

void
clock_hardtick()
{
    clock_ticks++;
}

/**
 * @brief Miêkkie tykniêcie.
 *
 * Procedura uruchamiana nazewn±trz obs³ugi przerwania zegara. Czas procesora
 * zajêty przez ni± opó¼nia kolejne jej wywo³anie, nie przerwania.
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


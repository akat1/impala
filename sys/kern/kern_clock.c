/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

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

/// Cz�stotliwo�� zegara systemowego
const int HZ = 100;

/// Inicjalizuje obs�ug� tykni�� zegara systemowego.
void
clock_init()
{
     spinlock_init(&soft_guard);
}


/**
 * Twarde tykni�cie zegara.
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
 * Mi�kkie tykni�cie.
 *
 * Procedura uruchamiana nazewn�trz obs�ugi przerwania zegara. Czas procesora
 * zaj�ty przez ni� op�nia kolejne jej wywo�anie, nie przerwania.
 */
void
clock_softtick()
{
    if ( spinlock_trylock(&soft_guard) ) {
        spinlock_unlock(&soft_guard);
        sched_action();
    }
}


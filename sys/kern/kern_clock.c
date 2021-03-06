/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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
#include <sys/time.h>
#include <sys/sched.h>
#include <sys/thread.h>
#include <sys/kmem.h>
#include <machine/io.h>
#include <machine/interrupt.h>
#include <machine/atomic.h>

/// Wirujący zamek zabezpieczający miękkie tyknięcie.
static spinlock_t soft_guard = { SPINLOCK_LOCK };

/// Licznik tyknięć.
volatile uint clock_ticks;
volatile timespec_t curtime;

/// Częstotliwość zegara systemowego
const int HZ = 100;
const int TICK = 1000000000/100; // 10^9 / HZ

const int mdays[] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 
                    365};

static void load_cmos_time(void);
static uint8_t read_bcd_cmos(uint8_t index);

static int callout_id = 0;
static bool is_less_callout(callout_t *x, callout_t *y);
static bool is_equal_id(callout_t *x, int id);
static list_t callouts;
static void handle_callouts(void);

uint8_t read_bcd_cmos(uint8_t index)
{
    io_out8(0x70, index);
    uint8_t r = io_in8(0x71);
    return ((r&0xf0)>>4)*10 + (r&0x0f);
}

#include <sys/utils.h>

/// Pobiera aktualny czas z NVRAM
void
load_cmos_time(void)
{
    int DAYS = 0;
    int yh = read_bcd_cmos(0x32);
    int yl = read_bcd_cmos(0x9);
    int year = yh*100+yl;
    int yearD = year - 1970;
    DAYS += yearD*365 + (yearD+1)/4 - (yearD+69)/100 + (yearD+369)/400;
    int mon = read_bcd_cmos(0x8);
    DAYS += mdays[mon] + ((mon>2 && !(year%4) && ((year%100) || !(year%400)))?
                                    1:0);
    int day = read_bcd_cmos(0x7);
    DAYS += day-1;
    int hou = read_bcd_cmos(0x4);
    int min = read_bcd_cmos(0x2);
    int sec = read_bcd_cmos(0x0);
    curtime.tv_sec = ((DAYS*24+hou)*60+min)*60+sec;
    curtime.tv_nsec = 0;
}

/// Inicjalizuje obsługę tyknięć zegara systemowego.
void
clock_init()
{
     spinlock_init(&soft_guard);
     load_cmos_time();
     list_create(&callouts, offsetof(callout_t, L_callouts), FALSE);
}


/**
 * Twarde tyknięcie zegara.
 *
 * Procedura uruchamiana wewnątrz obsługi przerwania zegara, nie może
 * opóźnić kolejnego tyknięcia.
 */
void
clock_hardtick()
{
    clock_ticks++;
    curtime.tv_nsec+=TICK;
    if(curtime.tv_nsec >= 1000000000) {
        curtime.tv_nsec -= 1000000000;
        curtime.tv_sec++;
    }
}

/**
 * Miękkie tyknięcie.
 *
 * Procedura uruchamiana nazewnątrz obsługi przerwania zegara. Czas procesora
 * zajęty przez nią opóźnia kolejne jej wywołanie, nie przerwania.
 */
void
clock_softtick()
{
    if ( spinlock_trylock(&soft_guard) ) {
        spinlock_unlock(&soft_guard);
        sched_action(); //nie dokonuje faktycznej zmiany wątku
        handle_callouts();
    }
}

static bool
is_less_callout(callout_t *x, callout_t *y)
{
    if ( x->timeout < y->timeout )
        return TRUE;
    else
        return FALSE;
}

static bool
is_equal_id(callout_t *x, int id)
{
    return (x->id == id);
}

static void
handle_callouts(void)
{
    callout_t *c;
    
    while(1) { 
        c = list_head(&callouts);

        if ( c == NULL )
            break;

        if ( clock_ticks >= c->timeout ) {
            (c->fn)(c->arg);
            list_remove(&callouts, c);
            kmem_free(c);
        } else
            break;
    }

    return;
}

/***
 * Procedura rejestrująca opóźnione wykonanie
 *
 * Interfejs calloutów wzorowany na SVR4
 */

int
clock_timeout(void (*fn)(void *), addr_t arg, uint delta)
{
    callout_t *nc = kmem_zalloc(sizeof(callout_t), KM_SLEEP);
    nc->id = callout_id;
    nc->fn = fn;
    nc->arg = arg;
    nc->timeout = clock_ticks + delta;

    list_insert_in_order(&callouts, nc, is_less_callout);

    return callout_id++;
}

/***
 * Procedura usuwająca opóźnione wykonanie z kolejki na podstawie id
 */

void
clock_untimeout(int id)
{
    callout_t *c;
    c = list_find(&callouts, is_equal_id, id);

    if ( c == NULL )
        return;

    list_remove(&callouts, c);

    return;
}

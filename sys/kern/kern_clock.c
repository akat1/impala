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
#include <sys/time.h>
#include <sys/sched.h>
#include <sys/thread.h>
#include <machine/io.h>
#include <machine/interrupt.h>
#include <machine/atomic.h>

/// Wiruj±cy zamek zabezpieczaj±cy miêkkie tykniêcie.
static spinlock_t soft_guard = { SPINLOCK_LOCK };

/// Licznik tykniêæ.
volatile uint clock_ticks;
volatile timespec_t curtime;

/// Czêstotliwo¶æ zegara systemowego
const int HZ = 100;
const int TICK = 1000000000/100; // 10^9 / HZ

const int mdays[] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 
                    365};

static void load_cmos_time(void);
static uint8_t read_bcd_cmos(uint8_t index);

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

/// Inicjalizuje obs³ugê tykniêæ zegara systemowego.
void
clock_init()
{
     spinlock_init(&soft_guard);
     load_cmos_time();
}


/**
 * Twarde tykniêcie zegara.
 *
 * Procedura uruchamiana wewn±trz obs³ugi przerwania zegara, nie mo¿e
 * opó¼niæ kolejnego tykniêcia.
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
 * Miêkkie tykniêcie.
 *
 * Procedura uruchamiana nazewn±trz obs³ugi przerwania zegara. Czas procesora
 * zajêty przez ni± opó¼nia kolejne jej wywo³anie, nie przerwania.
 */
void
clock_softtick()
{
    if ( spinlock_trylock(&soft_guard) ) {
        spinlock_unlock(&soft_guard);
        sched_action(); //nie dokonuje faktycznej zmiany w±tku
    }
}


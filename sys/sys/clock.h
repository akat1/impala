/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_CLOCK_H
#define __SYS_CLOCK_H

#ifdef __KERNEL

extern volatile uint clock_ticks;
extern const int HZ; //mo�e si� okaza�, �e tutaj lepiej da� #define

void clock_init(void);
void clock_softtick(void);
void clock_hardtick(void);

#endif
#endif


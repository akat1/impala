/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_i8254_H
#define __MACHINE_i8254_H

enum {
    PIT_CHAN0 = 0x40,
    PIT_CHAN1,
    PIT_CHAN2,
    PIT_MODE
};

enum {
    PIT_MAX_FREQ = 1193182
};

#ifdef __KERNEL

void i8254_init(void);
void i8254_set_freq(uint hz);

#endif
#endif


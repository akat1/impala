#ifndef __MACHINE_i8254_H
#define __MACHINE_i8254_H

enum {
    PIT_CHAN0 = 0x40,
    PIT_CHAN1,
    PIT_CHAN2,
    PIT_MODE
};


#ifdef __KERNEL

void i8254_init(void);
void i8254_set_freq(uint hz);

#endif
#endif


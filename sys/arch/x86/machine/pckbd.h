#ifndef __MACHINE_PCKBD_H
#define __MACHINE_PCKBD_H

  
#ifdef __KERNEL

extern int key_down[256];

void pckbd_init(void);
void set_kbd_repeat_rate(uint rate);
void set_kbd_delay(uint delay);

#endif
#endif


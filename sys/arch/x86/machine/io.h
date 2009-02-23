#ifndef __MACHINE_IO_H
#define __MACHINE_IO_H

#include <sys/types.h>

// Lepiej w funkcjach ni¿ makrach - kompilator sobie sprawdzi typy,
// a przy -O1 i tak powinien to zoptymalizowaæ do jednej instrukcji.

static inline uint32_t
io_in32(uint16_t port)
{
    uint val;
    __asm__ volatile("inl %%dx, %%eax":"=a" (val):"d" (port));
    return val;
}

static inline void
io_out32(uint16_t port, uint32_t val)
{
    __asm__ volatile("outl %%eax, %%dx"::"a" (val), "d" (port));
}

static inline uint16_t
io_in16(uint16_t port)
{
    uint16_t val;
    __asm__ volatile("inw %%dx, %%ax":"=a" (val):"d" (port));
    return val;
}

static inline void
io_out16(ushort port, uint16_t val)
{
    __asm__ volatile("outw %%ax, %%dx"::"a" (val), "d" (port));
}

static inline uint8_t
io_in8(uint8_t port)
{
    uint8_t val;
    __asm__ volatile("inb %%dx, %%al":"=a" (val):"d" (port));
    return val;
}


static inline void
io_out8(ushort port, uint8_t val)
{
    __asm__ volatile("outb %%al, %%dx"::"a" (val), "d" (port));
}

#endif



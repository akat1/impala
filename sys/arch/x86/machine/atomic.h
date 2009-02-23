#ifndef __MACHINE_ATOMIC_H
#define __MACHINE_ATOMIC_H

// TODO: sprawdziæ czy dobrze zrobi³em te OUTPUT:INPUT:CLOBBERS


// MPSAFE
static inline uint
atomic_change32(uint *addr, uint x)
{
  __asm__ volatile(
    "xchgl %0, %1"
    : "=r"(x), "=rg"(*addr)
    : "r"(x)
    : "memory"
  );
  return x;
}

// To chyba nie jest MPSAFE
static inline uint
atomic_fetch32(uint *addr)
{
    return *addr;
}

#endif


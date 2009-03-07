#ifndef __MACHINE_ATOMIC_H
#define __MACHINE_ATOMIC_H

// TODO: sprawdzi� czy dobrze zrobi�em te OUTPUT:INPUT:CLOBBERS


// MPSAFE
static inline uint
atomic_change_int(int *addr, int x)
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


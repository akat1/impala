/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_ATOMIC_H
#define __MACHINE_ATOMIC_H



// MPSAFE
static inline uint
atomic_change_int(volatile int *addr, int x)
{
  __asm__ volatile(
    "xchgl %0, %1"
    : "=a"(x), "=m"(*addr)
    : "a"(x)
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


/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_UTILS_H
#define __SYS_UTILS_H


/**
 * Funkcja wywoływana w sytuacjach awaryjnych.
 * Zatrzymuje system, wyświetlając podany komunikat.
 */

void panic(const char* msg, ...);


#define KASSERT(x) if(!(x)) \
    panic("Assertion ( %s ) failed in file: %s:%u, in function: %s", #x, __FILE__, __LINE__,  __func__);

#endif


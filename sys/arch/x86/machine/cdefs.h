/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_CDEFS_H
#define __MACHINE_CDEFS_H

typedef char *va_list;

#define VA_START(ap, s) ap = (char *) &s
#define VA_END(ap)
///@TODO: zrobiæ aby by³o co int bajtów.
#define VA_ARG(ap, type) *(type*)(ap += sizeof(type))

#define aligned(x) __attribute__((aligned(x)))


#endif


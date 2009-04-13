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
///@TODO: zrobi� aby by�o co int bajt�w.
#define VA_ARG(ap, type) *(type*)(ap += sizeof(type))

#define aligned(x) __attribute__((aligned(x)))


#endif


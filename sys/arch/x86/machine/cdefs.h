/*
 * ImpalaOS
 *  http://trzask.int.pl/impala/trac/
 *
 * $Id$
 */

#ifndef __MACHINE_CDEFS_H
#define __MACHINE_CDEFS_H

typedef char *va_list;

#define VA_START(ap, s) ap = (char *) &s
#define VA_END(ap)
#define VA_ARG(ap, type) *(type*)(ap += sizeof(type))



#endif


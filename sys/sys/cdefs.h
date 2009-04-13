/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_CDEFS_H
#define __SYS_CDEFS_H


#define __packed __attribute__((packed))

#ifndef NULL
#   define NULL (void*)0
#endif

// piekny triczek, rodem z Solarisa
#define offsetof(str, memb) ((size_t)(&(((str *)0)->memb)))

#define CXX_BEGIN   extern "C" {
#define CXX_END     }

#include <machine/cdefs.h>

#endif



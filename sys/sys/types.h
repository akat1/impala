#ifndef __SYS_TYPES_H
#define __SYS_TYPES_H

#include <sys/cdefs.h>


#include <machine/types.h>

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

#define addr_t void*
typedef int bool;
#define FALSE 0
#define TRUE  1

typedef struct thread thread_t;
typedef struct kthread kthread_t;
typedef struct spinlock spinlock_t;
typedef struct mutex mutex_t;
typedef struct condvar condvar_t;

#endif

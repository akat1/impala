/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

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


typedef uint32_t off_t;
typedef uint32_t dev_t;
typedef uint32_t blkno_t;
typedef int gid_t;
typedef int uid_t;
typedef int mode_t;

// Byæ mo¿e jaki¶ sprytny kod bêdzie definiowa³ typu o takich samych nazwach
#ifndef __HIDE_SYSTEM_TYPEDEFS
typedef struct thread thread_t;
typedef struct kthread kthread_t;
typedef struct spinlock spinlock_t;
typedef struct mutex mutex_t;
typedef struct semaph semaph_t;
typedef struct cqueue cqueue_t;
typedef struct proc proc_t;
typedef struct kmem_cache kmem_cache_t;
typedef struct device device_t;
typedef struct devd devd_t;
typedef struct devsw devsw_t;
typedef struct uio uio_t;
typedef struct filed filed_t;
typedef struct buf buf_t;
#endif

#include <sys/list.h>

#endif

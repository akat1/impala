/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
typedef int errno_t;
typedef uint time_t;
typedef uint mask_t;
typedef int key_t;
typedef int gid_t;
typedef int uid_t;
typedef uint dev_t;
typedef uint32_t ino_t;
typedef uint32_t blkno_t;
typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;
typedef uint mode_t;
typedef uint nlink_t;
typedef int pid_t;
typedef uintptr_t tid_t;
typedef void (*sighandler_t)(int);
typedef uint32_t sigset_t;

// Byæ mo¿e jaki¶ sprytny kod bêdzie definiowa³ typu o takich samych nazwach
#ifndef __HIDE_SYSTEM_TYPEDEFS
typedef struct thread thread_t;
typedef struct kthread kthread_t;
typedef struct spinlock spinlock_t;
typedef struct mutex mutex_t;
typedef struct semaph semaph_t;
typedef struct cqueue cqueue_t;
typedef struct sleepq sleepq_t;
typedef struct proc proc_t;
typedef struct kmem_cache kmem_cache_t;
typedef struct device device_t;
typedef struct devd devd_t;
typedef struct devsw devsw_t;
typedef struct uio uio_t;
typedef struct iobuf iobuf_t;
typedef struct vnode vnode_t;
typedef struct consdevsw consdevsw_t;
typedef struct timespec timespec_t;
typedef struct pcred pcred_t;
typedef struct filetable filetable_t;
typedef struct filetable_chunk filetable_chunk_t;
typedef struct file file_t;
typedef struct ipcmsq ipcmsq_t;
typedef struct biohash biohash_t;
typedef struct sigaction sigaction_t;

#include <sys/vm/vm_types.h>
#endif

#ifdef __KERNEL
#include <sys/list.h>
#endif


#endif

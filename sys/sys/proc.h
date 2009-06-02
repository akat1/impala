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

#ifndef __SYS_PROC_H
#define __SYS_PROC_H

#include <sys/thread.h>
#include <sys/vfs/vfs_node.h>
#include <sys/vfs/vfs_types.h>

struct pcred {
    uid_t           p_uid;       ///< identyfikator u¿ytkownika
    uid_t           p_euid;      ///< efektywny identyfikator u¿ytkownika
    gid_t           p_gid;       ///< identyfikator grupy
    gid_t           p_egid;      ///< efektywnty identyfikator grupy
    int             refcnt;      ///< licznik referencji
};

struct proc {
    pid_t           p_pid;       ///< identyfikator procesu
    pid_t           p_ppid;      ///< identyfikator rodzica
    pcred_t        *p_cred;      ///< przywileje
    filetable_t    *p_fd;        ///< pliki przypisane do procesu
    list_t          p_threads;   ///< lista w±tków wchodz±cych w sk³ad procesu
    list_t          p_children;  ///< lista dzieci procesu
    int             p_flags;     ///< flagi procesu
    int             p_status;    ///< status
    vm_space_t     *vm_space;    ///< przestrzeñ adresowa procesu
    vnode_t        *p_curdir;    ///< aktualny katalog procesu
    vnode_t        *p_rootdir;   ///< g³ówny katalog procesu (korzeñ)
    ipcmsq_t       *p_ipc_msq;   ///< prywatna kolejka wiadomo¶ci.
    mask_t          p_umask;     ///< maska tworzonych plików
    list_node_t     L_procs;     ///< wêze³ procesów
    list_node_t     L_children;  ///< wêze³ listy dzieci
};

#ifdef __KERNEL

/// Lista procesów dzia³aj±ych w systemie.
extern list_t procs_list;
/// Aktualnie wykonywany proces.
extern proc_t *initproc;
extern proc_t proc0;

void proc_init(void);
proc_t *proc_create(void);
thread_t * proc_create_thread(proc_t *, uintptr_t entry);
void proc_destroy_threads(proc_t *proc);
void proc_reset_vmspace(proc_t *proc);
void proc_insert_child(proc_t *proc, proc_t *child);
proc_t *proc_find(pid_t pid);
void proc_destroy(proc_t *p);
bool proc_is_zombie(proc_t *p);
bool proc_is_parent(proc_t *parent, proc_t *child);
int proc_fork(proc_t *p, proc_t **child);
void proc_exit(proc_t *p, int e);

enum PROC_FLAGS
{
    PROC_NEW    = 1<<0, // proces zosta³ utworzony
    PROC_ZOMBIE = 1<<1, // proces zostaje niszczony
    PROC_RUN    = 1<<2  // proces w kolejce uruchomieniowej
};

enum
{
    INIT_PID    = 0
};

#endif
#endif


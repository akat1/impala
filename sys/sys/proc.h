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

/** @file procesy
 */
#ifndef __SYS_PROC_H
#define __SYS_PROC_H

/// znaczniki procesu
enum PROC_FLAGS
{
    PROC_NEW        = 1<<0,         ///< proces został utworzony
    PROC_ZOMBIE     = 1<<1,         ///< proces zostaje niszczony
    PROC_RUN        = 1<<2,         ///< proces w kolejce uruchomieniowej
    PROC_AFTER_EXEC = 1<<3,         ///< proces wykonał już exec
    PROC_STOP       = 1<<4          ///< 
};

enum {
    INIT_PID =  1
};
/// opis działającego procesu dla użytkownika
struct procinfo {
    pid_t   pid;        ///< PID
    pid_t   ppid;       ///< PPID
    uchar   nice;       ///< priorytet procesu
    uchar   pri;
    int     threads;    ///< ilość wątków
    int     flags;      ///< znaczniki
    char    tty[10];    ///< terminal kontrolujący
    char    cmd[100];   ///< obraz
};

#ifdef __KERNEL
#include <sys/signal.h>
#include <sys/thread.h>
#include <sys/termios.h>
#include <sys/vfs/vfs_node.h>
#include <sys/vfs/vfs_types.h>
//^^^^^^^^^^^^^ ???

/// 
struct pcred {
    uid_t           p_uid;       ///< identyfikator użytkownika
    uid_t           p_euid;      ///< efektywny identyfikator użytkownika
    gid_t           p_gid;       ///< identyfikator grupy
    gid_t           p_egid;      ///< efektywnty identyfikator grupy
    int             refcnt;      ///< licznik referencji
};

/// deskryptor procesu
struct proc {
    pid_t           p_pid;       ///< identyfikator procesu
    pid_t           p_ppid;      ///< identyfikator rodzica
    pid_t           p_session;   ///< identyfikator sesji procesu
    pid_t           p_group;     ///< identyfikator grupy procesów
    tty_t          *p_ctty;      ///< terminal kontrolujący procesu
    pcred_t        *p_cred;      ///< przywileje
    filetable_t    *p_fd;        ///< pliki przypisane do procesu
    list_t          p_threads;   ///< lista wątków wchodzących w skład procesu
    list_t          p_children;  ///< lista dzieci procesu
    int             p_flags;     ///< flagi procesu
    int             p_status;    ///< status
    sigset_t        p_sig;       ///< maska sygnałów dostarczonych do procesu
    sigset_t        p_sigignore; ///< maska sygnałów ignorowanych
    sigaction_t     p_sigact[_NSIG]; ///< akcje sygnałów
    uchar           p_nice;      ///< poziom uprzejmości procesu
    uchar           p_ucpu;      ///< zużycie procesora
    uchar           p_pri;       ///< priorytet procesu
    vm_space_t     *vm_space;    ///< przestrzeń adresowa procesu
    vnode_t        *p_curdir;    ///< aktualny katalog procesu
    vnode_t        *p_rootdir;   ///< główny katalog procesu (korzeń)
    ipcmsq_t       *p_ipc_msq;   ///< prywatna kolejka wiadomości
    vm_addr_t       p_brk_addr;  ///< adres końca segmentu danych (*)
    sleepq_t        p_waitq;     ///< oczekiwanie na dzieci lub sygnały.
    mutex_t         p_mtx;       ///< zamek do synchronizacji
    mask_t          p_umask;     ///< maska tworzonych plików
    list_t          p_umtxs;     ///< lista zamków użytkownika
    list_node_t     L_procs;     ///< węzeł procesów
    list_node_t     L_children;  ///< węzeł listy dzieci
    const char     *p_cmd;
};

#define PROC_NZERO  20

extern list_t procs_list;
extern proc_t * volatile curproc;
extern proc_t * initproc;
extern proc_t proc0;

void proc_init(void);
proc_t *proc_create(void);
bool proc_has_thread(proc_t *, thread_t *ut);
void proc_destroy_threads(proc_t *proc);
void proc_reset_vmspace(proc_t *proc);
void proc_insert_child(proc_t *proc, proc_t *child);
proc_t *proc_find(pid_t pid);
void proc_destroy(proc_t *p);
void proc_delete(proc_t *proc);
bool proc_is_zombie(proc_t *p);
bool proc_is_parent(proc_t *parent, proc_t *child);
int proc_fork(proc_t *p, proc_t **child);
void proc_exit(proc_t *p, int e);
int proc_getinfos(int off, struct procinfo *info, int n);

thread_t * proc_create_thread(proc_t *, uintptr_t entry);
void proc_destroy_thread(proc_t *, thread_t *t);

mutex_t *proc_create_mutex(proc_t *);
int proc_destroy_mutex(proc_t *, mutex_t *);
bool proc_has_mutex(proc_t *, mutex_t *um);

#else

int getprocinfo(int off, struct procinfo *tab, int n);

#endif
#endif


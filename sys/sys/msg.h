/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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

/** @file kolejki wiadomości w Systemie piątym
 */
#ifndef __SYS_MSG_H
#define __SYS_MSG_H

/// deskryptor kolejek wiadomości w systemie piątym
struct msqid_ds {
    struct ipc_perm     msg_perm;           ///< prawa dostępu.
    msgqnum_t           msg_qnum;           
    msglen_t            msg_qbytes;
    pid_t               msg_lspid;
    pid_t               msg_lrpid;
    time_t              msg_stime, msg_rtime, msg_ctime;
};


#ifdef __KERNEL

/// kolejka wiadomości systemu piątego
struct ipcmsq {
    struct msqid_ds     msq_ds;         /// deskryptor użytkownika
    mutex_t             msq_mtx;        /// zamek do synchronizacji
    list_t              msq_data;       /// kolejka wiadomości
    int                 msq_refcnt;     /// ilość referencji
    bool                msq_working;    /// czy jest włączona
    key_t               msq_key;        /// klucz dostępu.
};

int ipc_msg_get(proc_t *proc, key_t key, int flags, int *id, ipcmsq_t **);
int ipc_msg_ctl(ipcmsq_t *, int, struct msqid_ds *);
int ipc_msg_snd(ipcmsq_t *, const void *, size_t , int );
int ipc_msg_rcv(ipcmsq_t *, void *, size_t, long type, int );
ipcmsq_t *ipc_msg_find(proc_t *p, int id);

#else /* __KERNEL */

int msgget(key_t , int);
int msgctl(int , int , struct msqid_ds *);
int msgrcv(int, void *, size_t, long, int);
int msgsnd(int, const void *, size_t , int );

#endif
#endif

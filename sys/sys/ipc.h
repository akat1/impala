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

#ifndef __SYS_IPC_H
#define __SYS_IPC_H

/*
 * POSIX nie wymaga pola key, ale jest ono na BSD. Zatem niektóre
 * programy mogą z tego korzystać.
 */

/// ochrona IPC systemu piątego
struct ipc_perm {
    uid_t       uid;        ///< obecny właściciel
    gid_t       gid;        ///< obecna grupa
    uid_t       cuid;       ///< twórca
    gid_t       cgid;       ///< grupa tworząca
    mode_t      mode;       ///< tryb dostępu
    key_t       key;        ///< klucz
};

typedef unsigned int    msgqnum_t;
typedef unsigned int    msglen_t;

#define IPC_CREAT   (1 << 20)
#define IPC_EXCL    (1 << 21)
#define IPC_NOWAIT  (1 << 22)

#define IPC_PRIVATE (0-1)

#define IPC_RMID    1
#define IPC_SET     2
#define IPC_STAT    3

#ifdef __KERNEL

void    sysvipc_init(void);

#else /* __KERNEL */
key_t   ftok(const char *, int);
#endif

#endif

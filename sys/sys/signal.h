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

#ifndef __SYS_SIGNAL_H
#define __SYS_SIGNAL_H

#define sigmask(X)      (1<<((X)-1))
#define sig_emptymask   0

#define SIG_DFL ((void*)0)  /* Domyślna akcja */
#define SIG_ERR ((void*)-1) /* Wartość błędu */
#define SIG_IGN ((void*)1)  /* Sygnał będzie ignorowany */

// SIGPROCMASK
#define SIG_SETMASK     0
#define SIG_BLOCK       1
#define SIG_UNBLOCK     2

struct sigaction {
    sighandler_t sa_handler;
    sigset_t     sa_mask;
    int          sa_flags;
};

// Sygnały
#define SIGHUP          1   ///< 
#define SIGINT          2   ///< przerwanie
#define SIGQUIT         3   ///< wyjście
#define SIGILL          4   ///< nieprawidłowa instrukcja
#define SIGTRAP         5   ///< 
#define SIGABRT         6   ///< abort
#define SIGIOT          6   ///< coś starożytnego
#define SIGBUS          7   ///< błąd szyny
#define SIGFPE          8   ///< błąd FPU
#define SIGKILL         9   ///< zabicie
#define SIGUSR1         10  ///< syg. użytkownika #1
#define SIGSEGV         11  ///< naruszenie ochrony pamięci
#define SIGUSR2         12  ///< syg. użytkownika #2
#define SIGPIPE         13  ///< zapis na nieczytany potok
#define SIGALRM         14  ///< 
#define SIGTERM         15  ///< zakończenie pracy
#define SIGSTKFLT       16  ///<
#define SIGCHLD         17  ///< dziecko unarło
#define SIGCONT         18  ///< kontynuuj działanie
#define SIGSTOP         19  ///< zatrzymaj działanie
#define SIGTSTP         20  ///< zatrzymaj działanie (gen. przez tty)
#define SIGTTIN         21  ///< 
#define SIGTTOU         22  ///<
#define SIGURG          23  ///< 
#define SIGXCPU         24  ///<
#define SIGXFSZ         25  ///<
#define SIGVTALRM       26  ///<
#define SIGPROF         27  ///<
#define SIGWINCH        28  ///< zmiana rozmiaru okna tty

#define _NSIG    32   

#define S_TERM      1<<0
#define S_ABORT     1<<1
#define S_IGNORE    1<<2
#define S_STOP      1<<3
#define S_CONT      1<<4

#ifdef __KERNEL
bool signal_present(proc_t *p);
int signal_proc(thread_t *t);
bool signal_send(proc_t *p, int sig);
bool signal_send_group(pid_t pgid, int sig);
void signal_handle(thread_t *t);
bool signal_ign_or_blk(proc_t *p, int sig);
#endif

#endif

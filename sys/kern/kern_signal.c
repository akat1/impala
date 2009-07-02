/*
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


#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/signal.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <machine/thread.h>
#include <machine/interrupt.h>
#include <sys/wait.h>

bool signal_present(proc_t *p);
int signal_proc(thread_t *t);
bool signal_send(proc_t *p, int sig);
void signal_handle(thread_t *p);
sighandler_t signal_action(proc_t *p, int sig);
int _get_signal(thread_t *p);
void _action_term(proc_t *p, int sig);
void _action_abort(proc_t *p, int sig);

int _sig_acts[] = {
    0,            /* NULL signal */
    S_TERM,       /* SIGHUP */
    S_TERM,       /* SIGINT */
    S_ABORT,      /* SIGQUIT */
    S_ABORT,      /* SIGILL */
    S_ABORT,      /* SIGTRAP */
    S_ABORT,      /* SIGABRT */
    S_IGNORE,     /* SIGIOT */
    S_ABORT,      /* SIGBUS */
    S_ABORT,      /* SIGFPE */
    S_TERM,       /* SIGKILL */
    S_TERM,       /* SIGUSR1 */
    S_ABORT,      /* SIGSEGV */
    S_TERM,       /* SIGUSR2 */
    S_TERM,       /* SIGPIPE */
    S_TERM,       /* SIGALRM */
    S_TERM,       /* SIGTERM */
    S_IGNORE,     /* SIGSTKFLT */
    S_IGNORE,     /* SIGCHLD */
    S_CONT,       /* SIGCONT */
    S_STOP,       /* SIGSTOP */
    S_STOP,       /* SIGTSTP */
    S_STOP,       /* SIGTTIN */
    S_STOP,       /* SIGTTOU */
    S_IGNORE,     /* SIGURG */
    S_ABORT,      /* SIGXCPU */
    S_ABORT,      /* SIGXFSZ */
    S_TERM,       /* SIGVTALRM */
    S_ABORT,      /* SIGPROF */
    S_IGNORE      /* SIGWINCH */
};

sighandler_t
signal_action(proc_t *p, int sig)
{
    return p->p_sigact[sig].sa_handler;
}


int
_get_signal(thread_t *t)
{
    unsigned int i;

    for ( i = 1 ; i <= _NSIG ; i++ ) {
        /* wybieramy sygnaly, ktore nie sa blokowane */
        if ( (t->thr_proc->p_sig & ~t->thr_sigblock) & sigmask(i) ) {
            t->thr_proc->p_sig &= ~sigmask(i);
            return i;
        }
    }

    return 0;
}

void
_action_abort(proc_t *p, int sig)
{
    /* TODO: narazie nie zrzucamy core */
    _action_term(p, MAKE_STATUS_SIGNALED(sig));
    return;
}

void
_action_term(proc_t *p, int sig)
{
    proc_exit(p, MAKE_STATUS_SIGNALED(sig));
    return;
}

bool
signal_present(proc_t *p)
{
    return (p->p_sig) ? TRUE : FALSE;
}

errno_t
signal_proc(thread_t *t)
{
    proc_t *p = t->thr_proc;
    int sig;

    sig = _get_signal(t);

    /* nie mamy nic do zrobienia */
    if ( sig == 0 ) {
        return -ENOMSG;
    }

    /* sprawdzamy czy sygna³ jest ignorowany lub blokowany */
    if ( sigmask(sig) & p->p_sigignore ) {
        return EOK;
    }

    /* uruchamiamy akcje dla sygna³u */
    
    if ( signal_action(p, sig) == SIG_DFL) {
        switch(_sig_acts[sig]) {
            case S_ABORT:
                _action_abort(p, sig);
                /* NOT REACHED */
            case S_TERM:
                _action_term(p, sig);
                /* NOT REACHED */
            case S_IGNORE:
                /* ignore */
                break;
            case S_STOP:
                /* XXX: stop */
                break;
            case S_CONT:
                /* CONT */
                break;
        }
    } else if ( signal_action(p, sig) == SIG_IGN ) {
        /* NOTHING TO DO */
    } else {
        return 0;
        thread_sigenter(curthread, signal_action(p, sig), sig);
    }

    return 0;
}

bool
signal_send(proc_t *p, int sig)
{
    if ( sig < 1 || sig > _NSIG )
        return FALSE;

    p->p_sig |= sigmask(sig) & (~p->p_sigignore);
    return TRUE;
}

bool
signal_ign_or_blk(proc_t *p, int sig)
{
    if(//(sigmask(sig) & p->p_sigblock) ||  // jak sprawdzaæ blokowanie?
       (sigmask(sig) & p->p_sigignore) ||
        signal_action(p, sig) == SIG_IGN) // niech kto¶ to zweryfikuje
        return TRUE;
    return FALSE;
}

bool
signal_send_group(pid_t pgid, int sig)
{
    if ( sig < 1 || sig > _NSIG )
        return FALSE;
    ///@todo signal_send_group -> tak, bzdura, trzeba to zmieniæ
    proc_t *p = NULL;
    while((p = list_next(&procs_list, p))) {
        if(p->p_group != pgid)
            continue;
        p->p_sig |= sigmask(sig) & (~p->p_sigignore);
    }
    return TRUE;
}

void
signal_handle(thread_t *t)
{
    if (ISUNSET(t->thr_flags, THREAD_USER)) return;
    while ( signal_present(t->thr_proc) ) {
        if ( signal_proc(t) != EOK ) {
            break;
        }
    }

    return;
}

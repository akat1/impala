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

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/errno.h>
#include <sys/utils.h>
#include <sys/syscall.h>
#include <sys/proc.h>
#include <sys/signal.h>

typedef struct kill_args kill_args_t;

struct kill_args {
    pid_t pid;
    int sig;
};

errno_t sc_kill(thread_t *p, syscall_result_t *r, kill_args_t *args);

errno_t
sc_kill(thread_t *p, syscall_result_t *r, kill_args_t *args)
{
    /* TODO:
     * 1) grupy procesow == -pid
     * 2) broadcast - pid == 0
     * 3) broadcast poza initem - pid == -1
     */

    proc_t *dest_proc = NULL;

    /* sprawdzamy sygnał */
    if ( args->sig < 0 || args->sig > _NSIG ) {
        r->result = -1;
        return EINVAL;
    }

    /* szukamy procesu, któremu mamy dostarczyć sygnał */
    dest_proc = proc_find(args->pid);

    if ( dest_proc == NULL && args->pid != 0 ) {
        r->result = -1;
        return ESRCH;
    }

    /* sprawdzamy czy możemy dostarczyć sygnał */
    /* ... */

    /* sprawdzamy komu dostarczamy sygnał */

    /* pojedynczy proces */
    if ( args->pid > 0 )
    {
        signal_send(dest_proc, args->sig);
        r->result = 0;
        return EOK;
    }

    /* Uzupelnic wg. TODO */
    r->result = -1;
    return -ENOSTR;
}


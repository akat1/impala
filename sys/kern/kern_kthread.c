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

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/kthread.h>
#include <sys/sched.h>
#include <sys/utils.h>

/// Wej¶ciowa procedura dla obs³ugi w±tku.
static void __kthr(kthread_t *arg);

/**
 * Tworzy nowy w±tek po stronie j±dra.
 * @param thr referencja do deskryptora w±tku.
 * @param f adres procedury wej¶ciowej.
 * @param arg adres przekazany jako argument do procedury wej¶ciowej.
 * 
 */
void
kthread_create(kthread_t *kthr, kthread_entry_f *f, void *arg)
{
    kthr->kt_arg = arg;
    kthr->kt_entry = f;
    kthr->kt_thread = thread_create(0, (void*)__kthr, kthr);
    kthr->kt_thread->thr_flags |= THREAD_KERNEL;
    sched_insert(kthr->kt_thread);
}

void
__kthr(kthread_t *arg)
{
//    TRACE_IN("elo");
//    TRACE_IN("arg=%p entry=%p entry_arg=%p", arg, arg->kt_entry, arg->kt_arg);
    arg->kt_entry(arg->kt_arg);
    sched_exit((thread_t *)arg);
}


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
#include <sys/proc.h>
#include <sys/sched.h>
#include <sys/string.h>
#include <sys/utils.h>
#include <sys/syscall.h>
#include <machine/thread.h>

errno_t sc_fork(thread_t *t, syscall_result_t *r);

thread_t *__copy_thread(thread_t *t);

void teest(void);

void teest(void)
{
    for(;;)
    kprintf("!"); 
}

thread_t *
__copy_thread(thread_t *t)
{
    thread_t *t_copy;
    /* XXX */
    t_copy = thread_create(t->thr_priv, teest, 0); //t->thr_entry_point, t->thr_entry_arg);
    mem_cpy(&(t_copy->thr_context), &(t->thr_context), sizeof(thread_context));
    t_copy->thr_flags = t->thr_flags & (~THREAD_SYSCALL);
    t_copy->thr_wakeup_time = t->thr_wakeup_time;
    mem_cpy(&(t_copy->thr_stack), &(t->thr_stack), THREAD_STACK_SIZE);
//    (t->thr_context).c_esp = 

    return t_copy;
}

errno_t
sc_fork(thread_t *t, syscall_result_t *r)
{
    proc_t *p;
    thread_t *thr, *thr_copy;

    /* tworzymy nowy proces */
    p = proc_create();
    p->p_ppid = t->thr_proc->p_pid;
    p->p_cred->p_uid = t->thr_proc->p_cred->p_uid;

    #define NEXT_THR() (thread_t*) list_next(&(t->thr_proc->p_threads), thr)

    thr = (thread_t *)list_head(&(t->thr_proc->p_threads));

    {
        thr_copy = __copy_thread(thr);
        thr_copy->thr_proc = p;
        proc_insert_thread(p, thr_copy);
        sched_insert(thr_copy);
    } while ( (thr = NEXT_THR()) );
    #undef NEXT_THR

    return EOK;
}


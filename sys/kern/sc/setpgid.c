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
#include <sys/syscall.h>

typedef struct setpgid_args setpgid_args_t;
struct setpgid_args {
    pid_t pid;
    pid_t pgid;
};

int sc_setpgid(thread_t *p, syscall_result_t *r, setpgid_args_t *args);

/// Przeniesienie procesu potomnego do innej grupy, w ramach tej samej sesji

int
sc_setpgid(thread_t *t, syscall_result_t *r, setpgid_args_t *args)
{
    proc_t *p = t->thr_proc;
    int pid = args->pid;
    int pgid = args->pgid;
    if(!pid)
        pid = p->p_pid;
    if(!pgid)
        pgid = pid;     //am i sure??
    if(pgid < 0)
        return -EINVAL;
    proc_t *target = proc_find(pid);
    if(!target)
        return -ESRCH;
    //liderem grupy jest proc z pid = 'pgid'; a co jak zgin±³?
    proc_t *dstleader = proc_find(pgid);
    if(!dstleader)
        return -ESRCH;//nie jestem pewien
    if(p!=target && !proc_is_parent(p, target))
        return -ESRCH;
    //jeste¶my tutaj -> target jest nami, lub naszym dzieckiem
    if(target->p_pid != p->p_pid) { //prawdziwy potomek
        if(ISSET(target->p_flags, PROC_AFTER_EXEC)) return -EACCES;
        if((target->p_session != p->p_session) || 
            (target->p_session == target->p_pid))
            return -EPERM;
    }
    if(target->p_session != dstleader->p_session)
        return -EPERM;
    target->p_group = pgid;
    r->result = 0;
    return -EOK;
}

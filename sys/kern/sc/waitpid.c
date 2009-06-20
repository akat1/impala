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
#include <sys/utils.h>
#include <sys/string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/proc.h>

typedef struct sc_waitpid_args sc_waitpid_args;

struct sc_waitpid_args {
    pid_t   pid;
    int     *status;
    int     options;

};

errno_t sc_waitpid(thread_t *p, syscall_result_t *r, sc_waitpid_args *args);
/** XXX: Narazie tylko per pid */

errno_t
sc_waitpid(thread_t *t, syscall_result_t *r, sc_waitpid_args *args)
{
    proc_t *p = t->thr_proc;
    if(args->pid == -1) {
        proc_t *p_iter;
        /* czekamy a¿, które¶ dziecko siê zakoñczy lub nadejdzie sygna³ */
        while(1)
        {
            p_iter = (proc_t *)list_head(&(p->p_children));
            /* proces nie ma dzieci - czekamy na sygnal */
            if ( p_iter == NULL ) {
                if(args->options & WNOHANG) {
                    ///libc musi to zapatchowaæ i je¶li jest NOHANG zwróciæ 0
                    return -ECHILD; 
                }
                for(;;);
            }
            #define NEXTPROC() (proc_t *)list_next(&p->p_children, p_iter)
            {
                if ( proc_is_zombie(p_iter) )
                {
                    // odlaczamy dziecko
                    list_remove(&(p->p_children), p_iter);
                    // zwracamy jego pid jako wynik
                    r->result = p_iter->p_pid;
                    // zwracamy status procesu
                    if ( args->status != NULL ) 
                        *(args->status) = p_iter->p_status;
                    // niszczymy dziecko
                    proc_delete(p_iter);
                    return -EOK;
                }
                if(ISSET(args->options, WUNTRACED) &&
                   ISSET(p_iter->p_flags, PROC_STOP)) {
                    r->result = p_iter->p_pid;
                    // zwracamy status procesu
                    if ( args->status != NULL ) 
                        *(args->status) = p_iter->p_status;
                    return -EOK;
                }
            } while ((p_iter = NEXTPROC()));
            #undef NEXTPROC
            if(ISSET(args->options, WNOHANG))
                return EOK; ///poprawne zachowanie?
        }
    }
    proc_t *to_trace = proc_find(args->pid);

    if ( to_trace == NULL || (!proc_is_parent(t->thr_proc, to_trace)))
        return -ECHILD;

    while(1)
    {
        if ( proc_is_zombie(to_trace) )
        {
            // odlaczamy dziecko
            list_remove(&(p->p_children), to_trace);
            // zwracamy jego pid jako wynik
            r->result = to_trace->p_pid;

            // zwracamy status procesu
            if ( args->status != NULL ) 
                *(args->status) = to_trace->p_status;

            // niszczymy dziecko
            proc_delete(to_trace);
            
            return -EOK;
        }
        if(ISSET(args->options, WUNTRACED) &&
           ISSET(to_trace->p_flags, PROC_STOP)) {
            r->result = to_trace->p_pid;
            // zwracamy status procesu
            if ( args->status != NULL ) 
                *(args->status) = to_trace->p_status;
            return -EOK;
        }
        if(args->options & WNOHANG)
            return EOK;
    }

    return -EOK;
}


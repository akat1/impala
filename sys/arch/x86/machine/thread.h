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

#ifndef __MACHINE_THREAD_H
#define __MACHINE_THREAD_H


typedef struct thread_context thread_context;
struct thread_context {
    uint32_t c_eax;
    uint32_t c_ebx;
    uint32_t c_ecx;
    uint32_t c_edx;
    uint32_t c_esi;
    uint32_t c_edi;
    uint32_t c_esp;
    uint32_t c_ebp;
    uint32_t c_eflags;
    uint32_t c_cr3;
    uint32_t c_eip;
    struct interrupt_frame *c_frame;
    ///@todo dodac koprocesor matematyczny
};

typedef struct signal_context signal_context;
struct signal_context {
    thread_context *context;
    signal_context *prev;
    sigset_t sigblock;
};

#ifdef __KERNEL

thread_context *thread_context_copy(thread_context *ctx);
void thread_sigenter(thread_t *t, addr_t proc, int signum);
void thread_sigreturn(thread_t *t);
void thread_context_destroy(thread_context *ctx);
void thread_context_load(thread_context *ctx);
int thread_context_store(thread_context *ctx);
void thread_context_init(thread_t *t, thread_context *ctx);
void thread_prepare(thread_t *c, vm_addr_t a, vm_addr_t e, vm_size_t o);
void thread_switch(thread_t *t_to, thread_t *t_from);
void thread_resume(thread_t *t);

#endif

#endif

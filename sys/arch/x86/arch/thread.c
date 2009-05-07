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
#include <sys/utils.h>
#include <sys/string.h>
#include <machine/cpu.h>

void setesp0(void* a);

/**
 * Inicjalizuje kontekst.
 * @param ctx referencja do kontekstu
 * @param priv poziom uprzywilejowania
 * @param ustack adres stosu
 */

void
thread_context_init(thread_context *ctx, int priv, addr_t ustack)
{
    mem_zero(ctx, sizeof(thread_context));
    ctx->c_eflags = EFLAGS_BITS;
    ctx->c_esp = (uint32_t) ustack + THREAD_STACK_SIZE-8;
}

/**
 * Prze��cza kontekst.
 * @param t_to deskryptor w�tku, do kt�rego trzeba si� prze��czy�.
 * @param t_from deskryptor obecnie dzia�aj�cego w�tku.
 */
void
thread_switch(thread_t *t_to, thread_t *t_from)
{
    if (thread_context_store(&t_from->thr_context)) {
        // Jestesmy w watku t_from
        curthread = t_to;
        setesp0(t_to->thr_kstack+0x2000);
        if (t_to->thr_flags & THREAD_NEW) {
            thread_enter(t_to);
        } else {
            thread_context_load(&t_to->thr_context);
        }
    } else {
        // Jestesmy ponownie w watku t_from
        curthread = t_from;
    }
}

/**
 * Uruchamia �wie�y kontekst.
 * @param t_to deskryptor w�tku.
 *
 * Procedura wchodzi w kod w�tku, kt�ry nie zosta� dot�d uruchomiony
 */

void
thread_enter(thread_t *t_to)
{

    typedef void (*entry_point)(void*);
    entry_point entry;
    t_to->thr_flags &= ~THREAD_NEW;
    entry = (entry_point) t_to->thr_entry_point;

    if (!(t_to->thr_flags & THREAD_KERNEL))
        cpu_user_mode();

    __asm__ volatile (
        "movl %%eax, %%esp"
        :
        : "a" (t_to->thr_context.c_esp)
        : "%esp" );
    entry(t_to->thr_entry_arg);
    panic("ERROR: should never be here! thread_enter/machine/thread.c\n");
}


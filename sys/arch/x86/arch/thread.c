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
#include <sys/vm.h>
#include <sys/kmem.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <sys/signal.h>
#include <machine/cpu.h>
#include <machine/descriptor.h>
#include <machine/interrupt.h>
#include <machine/i8259a.h>

void setesp0(void* a);
void __thread_enter(thread_t *t);
void __enter_arg_esp(void* entry, void* arg, uint32_t esp);

/**
 * Inicjalizuje kontekst.
 * @param ctx referencja do kontekstu
 */

void
thread_context_init(thread_t *t, thread_context *ctx)
{
    uintptr_t frame = (uintptr_t)t->thr_kstack;
    mem_zero(ctx, sizeof(thread_context));
    ctx->c_eflags = EFLAGS_BITS;
    ctx->c_frame = (interrupt_frame*)frame;
}

thread_context *
thread_context_copy(thread_context *ctx)
{
    thread_context *nctx = kmem_alloc(sizeof(thread_context), KM_SLEEP);
    interrupt_frame *nif = kmem_alloc(sizeof(interrupt_frame), KM_SLEEP);

    mem_cpy(nctx, ctx, sizeof(thread_context));
    mem_cpy(nif, ctx->c_frame, sizeof(interrupt_frame));

    nctx->c_frame = nif;

    return nctx;
}

void
thread_context_destroy(thread_context *ctx)
{
    kmem_free(ctx->c_frame);
    kmem_free(ctx);
    return;
}

void
thread_sigenter(thread_t *t, addr_t proc, int signum)
{
    thread_context *ctx = thread_context_copy(&(t->thr_context));
    signal_context *stx = kmem_alloc(sizeof(signal_context), KM_SLEEP);

    stx->context = ctx;
    stx->prev = t->thr_sigcontext;
    stx->sigblock = t->thr_sigblock;
    t->thr_sigblock = t->thr_proc->p_sigact[signum].sa_mask;
    t->thr_sigcontext = stx;

    mem_cpy((char *)t->thr_context.c_frame->f_esp-sizeof(int), &signum, sizeof(int));
    mem_cpy((char *)t->thr_context.c_frame->f_esp-sizeof(int)*2, &(t->thr_context.c_frame->f_eip), sizeof(int));
    t->thr_context.c_frame->f_esp -= 2*sizeof(int);
    t->thr_context.c_frame->f_eip = (uint32_t)proc;
    return;
}

void
thread_sigreturn(thread_t *t)
{
    signal_context *stx = t->thr_sigcontext;
    interrupt_frame *ifr;

    if ( stx == NULL ) {
        return;
    }

    t->thr_sigcontext = stx->prev;

    ifr = t->thr_context.c_frame;
    mem_cpy(&t->thr_context, stx->context, sizeof(thread_context));
    mem_cpy(t->thr_context.c_frame, stx->context->c_frame, sizeof(interrupt_frame));
    t->thr_context.c_frame = ifr;
    t->thr_sigblock = stx->sigblock;

    thread_context_destroy(stx->context);
    kmem_free(stx);

    return;
}

void
thread_prepare(thread_t *t, vm_addr_t av, vm_addr_t ev, vm_size_t off)
{
    uint32_t ESP = (uintptr_t)t->thr_stack + t->thr_stack_size - 4 - off;
    interrupt_frame *frame = t->thr_context.c_frame;
    mem_zero(t->thr_kstack, t->thr_kstack_size);
    if (t->thr_flags & THREAD_USER) {
        frame->f_cs = SEL_MK(SEL_UCODE, SEL_DPL3);
        frame->f_ds = SEL_MK(SEL_UDATA, SEL_DPL3);
        frame->f_es = SEL_MK(SEL_UDATA, SEL_DPL3);
        frame->f_fs = SEL_MK(SEL_UDATA, SEL_DPL3);
        frame->f_gs = SEL_MK(SEL_UDATA, SEL_DPL3);
        frame->f_ss = SEL_MK(SEL_UDATA, SEL_DPL3);
    }
    t->thr_context.c_esp = ESP;
    t->thr_context.c_eflags |= EFLAGS_IF|0x200;
    frame->f_eip = (uint32_t)t->thr_entry_point;
    frame->f_eflags = t->thr_context.c_eflags;
    frame->f_esp = ESP;
    frame->f_edi = av;
    frame->f_esi = ev;
#if 0
    kprintf("[%p] %p %p %p %p %p\n", frame, frame->f_cs, frame->f_ds, frame->f_eip,
        frame->f_esp, frame->f_ebp);
    kprintf(".\n");
#endif
}

uintptr_t
thread_get_pc(thread_t *t)
{
    return t->thr_context.c_frame->f_eip;
}

void
thread_fork(thread_t *t, thread_t *ct)
{
    uintptr_t frame = (uintptr_t)ct->thr_kstack;
    mem_cpy(&ct->thr_context, &t->thr_context, sizeof(t->thr_context));
    ct->thr_context.c_frame = (interrupt_frame*)frame;
    mem_cpy(ct->thr_context.c_frame, t->thr_context.c_frame,
        sizeof(interrupt_frame));
    ct->thr_stack = t->thr_stack;
    ct->thr_stack_size = t->thr_stack_size;
    ct->thr_context.c_frame->f_eax = 0;
    ct->thr_context.c_frame->f_ecx = 0;
    ct->thr_sigcontext = NULL;
}

/**
 * Prze³±cza kontekst.
 * @param t_to deskryptor w±tku, do którego trzeba siê prze³±czyæ.
 * @param t_from deskryptor obecnie dzia³aj±cego w±tku.
 */
void
thread_switch(thread_t * volatile t_to, thread_t * volatile t_from)
{
    if (t_from==NULL || thread_context_store(&t_from->thr_context)) {
        curthread = t_to;
        setesp0(t_to->thr_kstack + t_to->thr_kstack_size -4);
        if (t_to->thr_flags & THREAD_NEW) {
            thread_resume(t_to);
            if (0) __thread_enter(t_to);
        } else {
            thread_context_load(&t_to->thr_context);
        }
    } else {
        curthread = t_from;
    }
}


void
thread_resume(thread_t *t)
{
    CIPL = 0;
    i8259a_reset_mask();

    if (t->thr_flags & THREAD_NEW) {
        t->thr_flags &= ~THREAD_NEW;
    }
    vm_pmap_switch(&t->vm_space->pmap);
    if (t->thr_flags & THREAD_USER) {
        cpu_resume(t->thr_context.c_frame);
    } else {
        irq_enable();
        __asm__ volatile (
            "movl %1, %%esp;"
            "jmp *%0" :
            : "r"(t->thr_entry_point), "r"(t->thr_context.c_esp)
        );
    }
}

/**
 * Uruchamia ¶wie¿y kontekst.
 * @param t_to deskryptor w±tku.
 *
 * Procedura wchodzi w kod w±tku, który nie zosta³ dot±d uruchomiony
 */
void
__thread_enter(thread_t *t_to)
{
    typedef void (*entry_point)(void*);
    t_to->thr_flags &= ~THREAD_NEW;
            //kprintf("pmap: %p\n", &t_to->vm_space->pmap);

    vm_pmap_switch(&t_to->vm_space->pmap);
    void *arg = t_to->thr_entry_arg;
    entry_point entry = (entry_point) t_to->thr_entry_point;
    uint32_t ESP = (uintptr_t)t_to->thr_stack + t_to->thr_stack_size - 4;

    CIPL = 0;       // proces ma dzia³aæ z CIPL = 0
    extern void i8259a_reset_mask(void);
    i8259a_reset_mask();

    if (t_to->thr_flags & THREAD_USER) {
        cpu_user_mode();
    }
    else
        irq_enable();
    __enter_arg_esp(entry, arg, ESP); //na wszelki wypadek ;)
    panic("ERROR: should never be here! thread_enter/machine/thread.c\n");
}

void
__enter_arg_esp(void* entry, void* arg, uint32_t esp)
{
    typedef void (*entry_point)(void*);
    __asm__ volatile (
        "movl %%eax, %%esp"
        :
        : "a" (esp)
        : "%esp" );
    ((entry_point)entry)(arg);
}

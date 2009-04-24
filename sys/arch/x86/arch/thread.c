/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <machine/cpu.h>
#include <sys/libkutil.h>
#include <sys/kprintf.h>
#include <sys/utils.h>

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
 * Prze³±cza kontekst.
 * @param t_to deskryptor w±tku, do którego trzeba siê prze³±czyæ.
 * @param t_from deskryptor obecnie dzia³aj±cego w±tku.
 */
void
thread_switch(thread_t *t_to, thread_t *t_from)
{
    if (thread_context_store(&t_from->thr_context)) {
        // Jestesmy w watku t_from
        curthread = t_to;
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
 * Uruchamia ¶wie¿y kontekst.
 * @param t_to deskryptor w±tku.
 *
 * Procedura wchodzi w kod w±tku, który nie zosta³ dot±d uruchomiony
 */

void
thread_enter(thread_t *t_to)
{

    typedef void (*entry_point)(void*);
    entry_point entry;
    t_to->thr_flags &= ~THREAD_NEW;
    entry = (entry_point) t_to->thr_entry_point;
    __asm__ volatile (
        "movl %%eax, %%esp"
        :
        : "a" (t_to->thr_context.c_esp)
        : "%esp" );
    entry(t_to->thr_entry_arg);
    panic("ERROR: should never be here! thread_enter/machine/thread.c\n");
}


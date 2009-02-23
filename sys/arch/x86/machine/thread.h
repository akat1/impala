#ifndef __MACHINE_THREAD_H
#define __MACHINE_THREAD_H


typedef struct thread_context thread_context;
struct thread_context {
    uint32_t c_esp;
    uint32_t c_ebp;
    uint32_t c_eflags;
    uint32_t c_cr3;
    //TODO: dodac koprocesor matematyczny
};

#ifdef __KERNEL

void thread_context_load(thread_context *ctx);
int thread_context_store(thread_context *ctx);
void thread_context_init(thread_context *ctx, int priv, addr_t ustack);
void thread_enter(thread_t *t);
void thread_switch(thread_t *t_to, thread_t *t_from);

#endif



#endif

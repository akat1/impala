/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/kprintf.h>

typedef struct sc_exit_args sc_exit_args;
struct sc_exit_args {
	int error_code;
};

void sc_exit(thread_t *p, sc_exit_args *args);


void
sc_exit(thread_t *p, sc_exit_args *args)
{
    kprintf("TID(%u) exit %u\n", p->thr_tid, args->error_code);
    sched_exit();
}


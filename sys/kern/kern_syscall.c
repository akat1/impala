/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/syscall.h>

void sc_exit(thread_t *proc, va_list ap);
void sc_write(thread_t *prov, va_list ap);

typedef void sc_handler_f(thread_t *proc, va_list ap);

static sc_handler_f *syscall_table[] = {
    sc_exit,
    sc_exit,
    sc_exit,
    sc_write,
    sc_exit,
    sc_exit,
    sc_exit
};

void
syscall(thread_t *thr, int n, va_list ap)
{
    if (n < SYSCALL_MAX) {
        syscall_table[n](thr, ap);
    }
}


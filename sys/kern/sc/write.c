/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/kprintf.h>
#include <sys/libkutil.h>
#include <machine/video.h>

typedef struct sc_write_args sc_write_args;

struct sc_write_args {
    int fd;
    addr_t *data;
    size_t size;
};


void sc_write(thread_t *p, sc_write_args *args);

void
sc_write(thread_t *p, sc_write_args *args)
{
    char buf[128];
//     kprintf("TID(%u) - WRITE(%u,%p,%u)\n", p->thr_tid, args->fd, args->data, args->size);
    if (args->size > 128) args->size = 128;
//     mem_cpy(buf, args->data, args->size);
    buf[args->size] = 0;
    textscreen_enable_forced_attr(COLOR_BRIGHTGRAY);
    kprintf("%s\n", args->data);
    textscreen_disable_forced_attr();
}

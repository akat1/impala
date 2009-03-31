/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_PROC_H
#define __SYS_PROC_H

struct proc {
    int             p_pid;
    list_t          p_threads;
    int             p_flags;
    addr_t          p_entry;
    list_node_t     L_procs;
};

proc *proc_create();
void proc_exit();


#endif


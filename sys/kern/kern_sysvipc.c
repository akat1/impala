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
#include <sys/kernel.h>
#include <sys/ipc.h>
#include <sys/msg.h>

static void ipc_msg_init(void);


/*========================================================================
 * Komunikacja miêdzy procesami Systemu V
 */


void
sysvipc_init(void)
{
    ipc_msg_init();
}

/*========================================================================
 * Kolejki wiadomo¶ci
 */


kmem_cache_t *msg_cache;

typedef struct msqmsg msqmsg_t;
struct msqmsg {
    char        data[MSGMAX];
    long        type;
    size_t      size;
    list_node_t L_data;
};


static bool find_msq_eq_type(const msqmsg_t *msg, uintptr_t type);
static bool find_msq_le_type(const msqmsg_t *msg, uintptr_t type);
static void ipc_msg_create(ipcmsq_t *msq);

ipcmsq_t msgs[SYSVMSG_MAX];



void
ipc_msg_init()
{
    msg_cache = kmem_cache_create("sysvipc_msg", sizeof(msqmsg_t), NULL, NULL);
    for (int i = 0; i < SYSVMSG_MAX; i++) {
        ipc_msg_create(&msgs[i]);
    }
}

void
ipc_msg_create(ipcmsq_t *msq)
{
    LIST_CREATE(&msq->msq_data, msqmsg_t, L_data, FALSE);
    msq->msq_refcnt = 0;
    mutex_init(&msq->msq_mtx, MUTEX_CONDVAR);
}



ipcmsq_t *
ipc_msg_get(proc_t *p, key_t key, int flags, int *id)
{
    ipcmsq_t *msq;
    if (key == IPC_PRIVATE) {
        *id = SYSVMSG_MAX;
        if (p->p_ipc_msq) {
            msq = p->p_ipc_msq;
        } else
        if (flags & IPC_CREAT) {
            msq = p->p_ipc_msq = kmem_alloc(sizeof(ipcmsq_t), KM_SLEEP);
            ipc_msg_create(p->p_ipc_msq);
        }
    }
    return 0;
}

int
ipc_msg_ctl(ipcmsq_t *msq, struct msqid_ds *uds)
{
    return -ENOTSUP;
}

int
ipc_msg_snd(ipcmsq_t *msq, const void *uaddr, size_t size, int flags)
{
    int allocflags = (flags & IPC_NOWAIT) ? 0 : KM_SLEEP;
    if (size < sizeof(long)) {
        return -EINVAL;
    }
    mutex_lock(&msq->msq_mtx);
    msqmsg_t *msg = kmem_cache_alloc(msg_cache, allocflags);
    if (msg == NULL) {
        mutex_unlock(&msq->msq_mtx);
        return -EAGAIN;
    }
    copyin(msg->data, uaddr, MIN(size,MSGMAX));
    msg->size = MIN(size, MSGMAX);
    msg->type = * ((long*) msg->data);
    list_insert_tail(&msq->msq_data, msg);
    mutex_unlock(&msq->msq_mtx);
    return 0;
}


int
ipc_msg_rcv(ipcmsq_t *msq, void *uaddr, size_t size, long type, int flags)
{
    int err = -ENOMSG;
    msqmsg_t *msg = NULL;
    bool wait = flags & IPC_NOWAIT;
    if (size < sizeof(long)) {
        return -EINVAL;
    }
    mutex_lock(&msq->msq_mtx);
    if (type == 0) {
        while (wait && list_length(&msq->msq_data) == 0) {
            mutex_wait(&msq->msq_mtx);
        }
        msg = list_extract_first(&msq->msq_data);
        copyout(uaddr, msg->data, MIN(size, msg->size));
        kmem_cache_free(msg_cache, msg);
    } else
    if (type > 0) {
       msqmsg_t *msg = NULL;
       do {
            msg = list_find(&msq->msq_data, find_msq_eq_type, type);
        } while (wait && msg == NULL);
    } else {
       type = -type;
       msqmsg_t *msg = NULL;
       do {
            msg = list_find(&msq->msq_data, find_msq_le_type, type);
        } while (wait && msg == NULL);
    }
    ///@TODO MSG_NOERROR trzeba obs³ugiwaæ (a raczej nie obs³ugiwaæ)
    if (msg) {
        copyout(uaddr, msg->data, MIN(size,msg->size));
        err = 0;
        kmem_cache_free(msg_cache, msg);
    }
    mutex_unlock(&msq->msq_mtx);
    return err;
}


bool
find_msq_eq_type(const msqmsg_t *msg, uintptr_t type)
{
    return (msg->type == type);
}

bool
find_msq_le_type(const msqmsg_t *msg, uintptr_t type)
{
    return (msg->type <= type);
}


/*========================================================================
 * Kolejki wiadomo¶ci
 */



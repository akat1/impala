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

typedef struct ipckey ipckey_t;
struct ipckey {
    key_t key;
    int id;
    list_node_t L_keys;
};

static bool find_key_eq(const ipckey_t *key, key_t k);
static mutex_t key_lock;

void
sysvipc_init(void)
{
    mutex_init(&key_lock, MUTEX_NORMAL);
    ipc_msg_init();
}

bool
find_key_eq(const ipckey_t *key, key_t k)
{
    return (key->key == k);
}


///@todo sprawdzaæ prawa dostêpu.

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
static void msq_create(ipcmsq_t *msq);
static int msq_open(proc_t *, ipcmsq_t **, mode_t , key_t);
static void ipc_msg_destroy(ipcmsq_t *msq);
static void ipc_msg_flush(ipcmsq_t *msq);

ipcmsq_t msqs[SYSVMSG_MAX];
static list_t msq_keys;


void
ipc_msg_init()
{
    msg_cache = kmem_cache_create("sysvipc_msg", sizeof(msqmsg_t), NULL, NULL);
    for (int i = 0; i < SYSVMSG_MAX; i++) {
        msq_create(&msqs[i]);
    }
    LIST_CREATE(&msq_keys, ipckey_t, L_keys, FALSE);
}

void
msq_create(ipcmsq_t *msq)
{
    LIST_CREATE(&msq->msq_data, msqmsg_t, L_data, FALSE);
    msq->msq_refcnt = 0;
    msq->msq_working = FALSE;
    mutex_init(&msq->msq_mtx, MUTEX_CONDVAR);
}


ipcmsq_t *
ipc_msg_find(proc_t *p, int id)
{
    ipcmsq_t *m = NULL;
    if (id < 0 && SYSVMSG_MAX < id) return NULL;
    mutex_lock(&key_lock);
    if (id == SYSVMSG_MAX)
        m = p->p_ipc_msq;
        else m = &msqs[id];
    if (m && !m->msq_working) m = NULL;
    mutex_unlock(&key_lock);
    return m;
}

void
ipc_msg_flush(ipcmsq_t *msq)
{
    msqmsg_t *msg = NULL;
    while ( (msg = list_next(&msq->msq_data, msg)) ) {
        kmem_cache_free(msg_cache, msg);
    }
    msq->msq_working = FALSE;
}

void
ipc_msg_destroy(ipcmsq_t *msq)
{
    mutex_lock(&msq->msq_mtx);
    KASSERT(msq->msq_refcnt > 0);
    msq->msq_refcnt--;
    mutex_unlock(&msq->msq_mtx);
    if (msq->msq_refcnt == 0) {
        if (msq->msq_key == IPC_PRIVATE) {

        }
    }
}

int
msq_open(proc_t *p, ipcmsq_t **m, mode_t mode, key_t k)
{
    for (int i = 0; i < SYSVMSG_MAX; i++)
        if (!msqs[i].msq_working) {
            msqs[i].msq_working = TRUE;
            *m = &msqs[i];
            struct ipc_perm *ip = &msqs[i].msq_ds.msg_perm;
            ip->cuid = p->p_cred->p_euid;
            ip->uid = p->p_cred->p_euid;
            ip->mode = mode;
            ip->cgid = p->p_cred->p_egid;
            ip->gid = p->p_cred->p_egid;
            ip->key = k;
            ipckey_t *ik = kmem_alloc(sizeof(*ik), KM_SLEEP);
            ik->key = k;
            ik->id = i;
            list_insert_tail(&msq_keys, ik);
            TRACE_IN("found %u", i);
            return i;
        }
    return -1;
}

int
ipc_msg_get(proc_t *p, key_t key, int flags, int *id, ipcmsq_t **r)
{
    TRACE_IN("p=%p key=%p flags=%p", p, key, flags & ~0777);
    TRACE_IN("flags<%s,%s>",
            (flags & IPC_CREAT)? "IPC_CREAT" : "",
            (flags & IPC_EXCL)? "IPC_EXCL" : ""
    );
    int err = 0;
    int mid = -1;
    ipcmsq_t *msq = 0;
    if (key == IPC_PRIVATE) {
        mid = SYSVMSG_MAX;
        if (p->p_ipc_msq && flags & IPC_EXCL) {
            err = -EEXIST;
        }
        if (p->p_ipc_msq) {
            msq = p->p_ipc_msq;
        } else
        if (flags & IPC_CREAT) {
            msq = p->p_ipc_msq = kmem_alloc(sizeof(ipcmsq_t), KM_SLEEP);
            msq_create(p->p_ipc_msq);
            p->p_ipc_msq->msq_key = key;
        } else err = -ENOENT;
    } else {
        mutex_lock(&key_lock);
        ipckey_t *ipck = list_find(&msq_keys, find_key_eq, key);
        TRACE_IN("ipck = %p", ipck);
        if (ipck && flags & IPC_EXCL) {
            TRACE_IN("exlusive!");
            err = -EEXIST;
        } else
        if (ipck) {
            TRACE_IN("OK");
            mid = ipck->id;
            msq = &msqs[ipck->id];
        } else
        if (flags & IPC_CREAT) {
            TRACE_IN("open!");
            mid = msq_open(p, &msq, flags & 0777, key);
            if (mid == -1) err = -ENOSPC;
        } else {
            TRACE_IN("not found");
            err = -ENOENT;
        }
        mutex_unlock(&key_lock);
    }
    *id = mid;
    *r = msq;
    return err;
}

int
ipc_msg_ctl(ipcmsq_t *msq, int cmd, struct msqid_ds *uds)
{
    int err = 0;
    mutex_lock(&msq->msq_mtx);
    if (cmd == IPC_STAT) {
        copyout(uds, &msq->msq_ds, sizeof(msq->msq_ds));
    } else
    if (cmd == IPC_SET) {
        err = -ENOTSUP;
    } else
    if (cmd == IPC_RMID) {
        ipc_msg_flush(msq);
        mutex_unlock(&msq->msq_mtx);
        ipc_msg_destroy(msq);
        return 0;
    }
    mutex_unlock(&msq->msq_mtx);

    return err;
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
    ///@todo MSG_NOERROR trzeba obs³ugiwaæ (a raczej nie obs³ugiwaæ)
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


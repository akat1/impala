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
#include <sys/errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "pthread_priv.h"

static void __entry(void)
{
    pthread_t t = thr_getarg();
    PTHREAD_LOG("new thread started pthread=%p(tid=%p)", t, t->pth_id);
    pthread_exit( t->pth_entry(t->pth_entry_arg) );
}


/**
 * Tworzy nowy w±tek u¿ytkownika.
 * @param pres wska¼nik na deskryptor w±tku (do wype³nienia).
 * @param attr atrybuty w±tku (NULL dla domy¶lnych).
 * @param entry adres procedury wej¶ciowej.
 * @param arg argument procedury wej¶ciowej.
 *
 */
int
pthread_create(pthread_t *pres, const pthread_attr_t *attr,
            pthread_entry entry, void *arg)
{
    __PTHREAD_INITIALIZE();
    PTHREAD_LOG("creting new POSIX thread entry=%p arg=%p", entry, arg);
    pthread_t p = malloc( sizeof(struct pthread) );
    if (attr == NULL) {
        pthread_attr_init(&p->pth_attr);
    } else {
        p->pth_attr = *attr;
    }
    p->pth_entry = entry;
    p->pth_entry_arg = arg;

    void *stack_addr;
    size_t stack_size;
    pthread_attr_getstackaddr(&p->pth_attr, &stack_addr);
    pthread_attr_getstacksize(&p->pth_attr, &stack_size);
    p->pth_id = thr_create(__entry, stack_addr, stack_size, p);
    if (p->pth_id == -1) {
        // zwolniæ pamiêæ
        return -1;
    }
    *pres = p;   
    return 0;
}

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

#ifndef __SYS_SCHED_H
#define __SYS_SCHED_H

#ifdef __KERNEL
extern int sched_quantum;

void sched_init(void);
void sched_action(void);
void try_sched_yield(void);
void do_switch(void);
void sched_yield(void);
void sched_exit(thread_t *thr);
void sched_exit_1(thread_t *thr);
void sched_exit_2(thread_t *thr);

void sched_insert(thread_t *thr);
void sched_remove(thread_t *thr);

void sched_unlock_and_wait(mutex_t *m);
#define SCHED_WAIT(d) sched_wait(__FILE__,__func__,__LINE__, (d))
void sched_wait(const char *fl, const char *fn, int l, const char *d);
void sched_wakeup(thread_t *n);

#define IMSLEEP(ms, d) imsleep(ms, __FILE__, __func__, __LINE__, d)
#define MSLEEP(t,d) msleep(t, __FILE__,__func__, __LINE__, d)
#define SSLEEP(t,d) ssleep(t, __FILE__,__func__, __LINE__, d)
void imsleep(uint mtime, const char *, const char *, int, const char *);
void msleep(uint mtime, const char *fl, const char *fn, int l, const char *d);
void ssleep(uint stime, const char *fl, const char *fn, int l, const char *d);


/* Ilo¶æ kolejek dla procesów */
#define SCHED_NQ            32
/* Ilo¶æ priorytetów na kolejke */
#define SCHED_PQ            4
/* Ilo¶æ kwantów czasu pomiêdzy kolejnymi reorganizacjami kolejek */
#define SCHED_RESCHEDULE    100
/* Priorytet procesu, je¿eli dostanie procesor */
#define SCHED_UCPU_MAX      128

#endif



#endif


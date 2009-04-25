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

#ifndef __MACHINE_TSS_H
#define __MACHINE_TSS_H

typedef struct task_state_segment task_state_segment;
struct task_state_segment {
    uint32_t tss_backlink;
    uint32_t tss_esp0;
    uint32_t tss_ss0;
    uint32_t tss_esp1;
    uint32_t tss_ss1;
    uint32_t tss_esp2;
    uint32_t tss_ss2;
    uint32_t tss_cr3;
    uint32_t tss_eip;
    uint32_t tss_eflags;
    uint32_t tss_eax;
    uint32_t tss_ecx;
    uint32_t tss_edx;
    uint32_t tss_ebx;
    uint32_t tss_esp;
    uint32_t tss_ebp;
    uint32_t tss_esi;
    uint32_t tss_edi;
    uint32_t tss_es;
    uint32_t tss_cs;
    uint32_t tss_ss;
    uint32_t tss_ds;
    uint32_t tss_fs;
    uint32_t tss_gs;
    uint32_t tss_ldtr;
    uint32_t tss_io;
} __packed;


#endif



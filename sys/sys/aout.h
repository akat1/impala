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

#ifndef __SYS_AOUT_H
#define __SYS_AOUT_H

typedef struct exec exec_t;
struct exec {
    uint32_t    a_midmag;
    uint32_t    a_text;
    uint32_t    a_data;
    uint32_t    a_bss;
    uint32_t    a_syms;
    uint32_t    a_entry;
    uint32_t    a_trsize;
    uint32_t    a_drsize;
};

typedef struct nlist nlist_t;
struct nlist {
    union {
        char    *n_name;
        long    n_strx;
    } n_un;
    int8_t      n_type;
    int8_t      n_other;
    int16_t     n_desc;
    uint32_t    n_value;
};

#define OMAGIC      0407
#define NMAGIC      0410
#define ZMAGIC      0413
#define QMAGIC      0314
#define MID_ZERO    0
#define EX_PIC      0x10
#define EX_DYNAMIC  0x20

#define N_UNDF      0
#define N_ABS       2
#define N_TEXT      4
#define N_DATA      6
#define N_BSS       8
#define N_EX        1

#define N_GETMAGIC(ex) (((ex).a_midmag) & 0xffff)
#define N_GETMID(ex) (((ex).a_midmag >> 16) & 0x3ff)
#define N_GETFLAG(ex) (((ex).a_midmag >> 26) & 0x3f)
#define N_BADMAG(ex) (N_GETMAGIC(ex) != ZMAGIC)


#define N_TXTOFF(ex) sizeof(exec_t)
#define N_DATAOFF(ex) (N_TXTOFF(ex) + (ex).a_text)
#define N_RELOFF(ex) (N_DATAOFF(ex) + (ex).a_data)
#define N_SYMOFF(ex) (N_RELOFF(ex) + (ex).a_trsize + (ex).a_drsize)
#define N_STROFF(ex) (N_SYMOFF(ex) + (ex).a_syms)



#ifdef __KERNEL
bool aout_check(const void *first_page);
#endif
#endif
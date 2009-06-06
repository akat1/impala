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

#ifndef __MACHINE_BUS_ISA_H
#define __MACHINE_BUS_ISA_H
#ifdef __KERNEL

typedef struct bus_isa_dma bus_isa_dma_t;

enum {
    ISA_DMA_8,
    ISA_DMA_16
};

enum {
    ISA_DMA_FDC = 0x2
};

enum {
    ISA_DMA_MODE_SINGLE = 1 << 6,
    ISA_DMA_MODE_AUTO = 1 << 4,
    ISA_DMA_MODE_READ = 1 << 2,
    ISA_DMA_MODE_WRITE = 2 << 2,
    ISA_DMA_MODE_INVLD = 3 << 2,
};

void bus_isa_init(void);
bus_isa_dma_t *bus_isa_dma_alloc(int chan);
void bus_isa_dma_write(bus_isa_dma_t *d, void *buf, size_t size);
void bus_isa_dma_read(bus_isa_dma_t *d, void *buf, size_t size);
void bus_isa_dma_finish(bus_isa_dma_t *ch);


#endif
#endif

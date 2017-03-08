/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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
typedef struct bus_isa_pnp bus_isa_pnp_t;

enum {
    ISA_DMA_FDC = 0x02
};

enum {
    ISA_DMA_WRITE = 0x08, 
    ISA_DMA_SINGLE = 0x40,
    ISA_DMA_AUTO = 0x10,
    ISA_DMA_READ = 0x04
};

void bus_isa_init(void);

bus_isa_dma_t *bus_isa_dma_alloc(int chan);
void bus_isa_dma_free(bus_isa_dma_t *ch);
int bus_isa_dma_prepare(bus_isa_dma_t *d, int cmd, void *buf, size_t size);
void bus_isa_dma_finish(bus_isa_dma_t *d);

struct bus_isa_pnp {
    uint32_t    name;
    uint32_t    rev;
    uint32_t    serial;
    int irq;
    int io;
};

int bus_isa_probe(bus_isa_pnp_t *);

#endif
#endif

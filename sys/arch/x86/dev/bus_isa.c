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

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/string.h>
#include <sys/vm.h>
#include <machine/bus/isa.h>
#include <machine/interrupt.h>
#include <machine/io.h>

enum {
    DMA_BUFSIZE = 0x10000,
};


typedef struct i8237A_chip i8237A_chip_t;
struct i8237A_chip {
    int     stat_reg;
    int     cmd_reg;
    int     req_reg;
    int     chmask_reg;
    int     chmode_reg;
    int     word_reg;
    int     intm_reg;
    int     mask_reg;
};

struct bus_isa_dma {
    vm_paddr_t       bufpaddr;
    void            *bufaddr;
    size_t           bufsize;
    void            *req_addr;
    size_t           req_size;
    bool             used;
    int              idx;
    int              mode;
    i8237A_chip_t   *bus;
};

static i8237A_chip_t dma_chips[] = {
    { 0x08, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F }, // chip #1
    { 0xD0, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDE }  // chip #2
};

static int dma_page_regs[] = {
    0x00, 0x02, 0x04, 0x06, 0xc0, 0xc2, 0xc4, 0xc6
};

static int dma_count_regs[] = {
    0x01, 0x03, 0x05, 0x07, 0xc1, 0xc3, 0xc5, 0xc7
};

static int dma_external_regs[] = {
    0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a
};




#define IO_STAT(ch) ch->bus->stat_reg
#define IO_CMD(ch) ch->bus->cmd_reg
#define IO_REQ(ch) ch->bus->req_reg
#define IO_CHMASK(ch) ch->bus->chmask_reg
#define IO_CHMODE(ch) ch->bus->chmode_reg
#define IO_WORD(ch) ch->bus->word_reg
#define IO_INTM(ch) ch->bus->intm_reg
#define IO_MASK(ch) ch->bus->mask_reg
#define IO_PADDR(ch) dma_page_regs[ch->idx]
#define IO_COUNT(ch) dma_count_regs[ch->idx]
#define IO_EXTRN(ch) dma_external_regs[ch->idx]
#define _IDX(ch) (ch->idx%4)
#define STCL 4

enum {
    ISA_PNP_IO  = 0x279,
    ISA_PNP_WR  = 0x2A9
};


static void pnp_init(void);
static void dma_init(void);
static bus_isa_dma_t channels[8];


void
bus_isa_init()
{
    dma_init();
    pnp_init();
}

/*========================================================================
 * Obsługa ISA-PNP
 */

void
pnp_init()
{

}

int
bus_isa_probe(bus_isa_pnp_t *info)
{
    return 0;
}

/*========================================================================
 * Obsługa ISA-DMA
 */

void
dma_init()
{
    io_out8(0xd8, 0xff);
    io_out8(0x0d, 0xff);
    io_out8(0xdc, 0xff);
    for (int i = 0; i < 8; i++) {
        if (i==0 || i==4 ) {
            channels[i].used = TRUE;
            continue;
        }
        channels[i].used = FALSE;
        channels[i].bufsize = DMA_BUFSIZE;
        channels[i].bufpaddr = DMA_BUFSIZE*(i+1);
        channels[i].bufaddr = NULL;
        channels[i].idx = i;
        channels[i].bus = &dma_chips[i>4];
    }
}



bus_isa_dma_t *
bus_isa_dma_alloc(int chan)
{
    if (7 < chan) return NULL;
    if (channels[chan].used) return NULL;
    bus_isa_dma_t *ch = &channels[chan];
    vm_physmap(ch->bufpaddr, ch->bufsize, &ch->bufaddr);

    DEBUGF("alloc DMA chan %u physmem %p+%p virtmem %p",
        chan, ch->bufpaddr, ch->bufsize, ch->bufaddr);

    return ch;

}

void
bus_isa_dma_free(bus_isa_dma_t *ch)
{
    DEBUGF("free DMA chan %u physmem %p+%p virtmem %p",
        ch->idx, ch->bufpaddr, ch->bufsize, ch->bufaddr);
    vm_unmap((vm_addr_t)ch->bufaddr, ch->bufsize);
    ch->bufaddr = NULL;
    ch->used = FALSE;
}

int
bus_isa_dma_prepare(bus_isa_dma_t *ch, int cmd, void *reqbuf, size_t size)
{
    enum {
        mode = ISA_DMA_SINGLE
    };
    uint8_t command;
    ch->mode = cmd;
    ch->req_addr = reqbuf;
    ch->req_size = size;
    command = cmd|mode|_IDX(ch);
    if (size > ch->bufsize) return -1;
    size--;
    int s = splhigh();
    io_out8(IO_CHMASK(ch),  _IDX(ch) | STCL);
    io_out8(IO_WORD(ch),    0xff);
    io_out8(IO_PADDR(ch),   (ch->bufpaddr >>  0) & 0xff);
    io_out8(IO_PADDR(ch),   (ch->bufpaddr >>  8) & 0xff);
    io_out8(IO_EXTRN(ch),   (ch->bufpaddr >> 16) & 0xff);
    io_out8(IO_WORD(ch),    0xff);
    io_out8(IO_COUNT(ch),   size & 0xff);
    io_out8(IO_COUNT(ch),   (size >> 8) & 0xff);
    io_out8(IO_CHMODE(ch),  command);
    io_out8(IO_CHMASK(ch),  _IDX(ch));

    if (ch->mode & ISA_DMA_WRITE) {
        memcpy(ch->bufaddr, ch->req_addr, ch->req_size);
    }

    splx(s);
    return 0;
}

void
bus_isa_dma_finish(bus_isa_dma_t *ch)
{
    if (ch->mode & ISA_DMA_READ) {
        memcpy(ch->req_addr, ch->bufaddr, ch->req_size);
    }
    ch->mode = 0;
}



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
 * $Id: bus_isa.c 155 2009-04-28 12:50:35Z wieczyk $
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/string.h>
#include <machine/bus/isa.h>
#include <machine/io.h>


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
    0x87, 0x83, 0x82, 0x81, 0x8f, 0x8b, 0x89, 0x8a
};

enum {
    DMA_BUFSIZE = 0x10000
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


static void dma_init(void);

static bus_isa_dma_t channels[8];

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
        channels[i].bus = &dma_chips[i%4];
    }
}


void
bus_isa_init()
{
    dma_init();
}


bus_isa_dma_t *
bus_isa_dma_alloc(int chan)
{
    if (7 < chan) return NULL;
    if (channels[chan].used) return NULL;
    bus_isa_dma_t *ch = &channels[chan];
    uint16_t page = (uint32_t)ch->bufpaddr >> 0x12;
    ch->used = TRUE;
    io_out8(IO_CHMASK(ch), _IDX(ch) | STCL);
    io_out8(0xd8, 0xff);
    io_out8(IO_PADDR(ch), page & 0xff);
    io_out8(IO_PADDR(ch), (page >> 8) & 0xff);
    io_out8(IO_EXTRN(ch), 0x00);
    io_out8(IO_CHMASK(ch), _IDX(ch) );
    return &channels[chan];
}

///@brief

void
bus_isa_dma_read(bus_isa_dma_t *ch, void *reqbuf, size_t size)
{
    enum {
        mode = (ISA_DMA_MODE_READ|ISA_DMA_MODE_SINGLE|ISA_DMA_MODE_AUTO)
    };
    ch->mode = ISA_DMA_MODE_READ;
//     ch->size = size;
    io_out8(IO_CHMASK(ch), _IDX(ch) | STCL);
    io_out8(0xd8, 0xff);
    io_out8(IO_COUNT(ch), size & 0xff);
    io_out8(IO_COUNT(ch), (size >> 8) & 0xff);
    io_out8(IO_CHMODE(ch), mode + _IDX(ch));
    io_out8(IO_CHMASK(ch), _IDX(ch) );
    DEBUGF("Channel %u prepared for reading", ch->idx);
}

void
bus_isa_dma_write(bus_isa_dma_t *ch, void *reqbuf, size_t size)
{
    enum {
        mode = (ISA_DMA_MODE_WRITE|ISA_DMA_MODE_SINGLE|ISA_DMA_MODE_AUTO)
    };
    ch->mode = ISA_DMA_MODE_WRITE;
//     ch->size = size;
//     mem_cpy(ch->addr, ch->req_addr, ch->size);
    io_out8(IO_CHMASK(ch), _IDX(ch) | STCL);
    io_out8(0xd8, 0xff);
    io_out8(IO_COUNT(ch), size & 0xff);
    io_out8(IO_COUNT(ch), (size >> 8) & 0xff);
    io_out8(IO_CHMODE(ch), mode + _IDX(ch));
    io_out8(IO_CHMASK(ch), _IDX(ch) );
    DEBUGF("Channel %u prepared for writing", ch->idx);
}

void
bus_isa_dma_finish(bus_isa_dma_t *ch)
{
    if (ch->mode == ISA_DMA_MODE_READ) {
//         mem_cpy(ch->req_addr, ch->bufaddr, ch->bufsize);
    }
    ch->mode = 0;
    DEBUGF("Finished transfer at channel %u", ch->idx);
}


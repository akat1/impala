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
#include <sys/vm.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/utils.h>

static bool has_hole_after_reg(const vm_region_t *reg, vm_size_t size);
static bool is_prev(const vm_region_t *regA, const vm_region_t *regB);
static void expand_region(vm_segment_t *, vm_region_t *, vm_size_t);
static bool is_containing_addr(const vm_region_t *reg, vm_addr_t addr);

void
vm_segment_create(vm_segment_t *vseg, vm_space_t *vsp,
    vm_addr_t base, vm_size_t size, vm_size_t limit)
{
    vseg->space = vsp;
    vseg->base = base;
    vseg->size = size;
    vseg->end = vseg->base + vseg->size;
    vseg->limit = limit;
    list_create(&vseg->regions, offsetof(vm_region_t,L_regions), FALSE);
}




int
vm_segment_alloc(vm_segment_t *vseg, vm_size_t size, void *_res)
{
    vm_addr_t *res = _res;
    size = PAGE_ROUND(size);
    vm_region_t *region;
    // zawsze szuakamy dziur za jakim¶ regionem, dziura nigdy nie wyst±pi
    // przed pierwszym regionem, bo na pocz±tku semestru s± strony przydzielone
    // przy inicjalizacji VM - które nigdy nie bêd± zwolnione.
    region = list_find(&vseg->regions, has_hole_after_reg, size);
    KASSERT(region != NULL);
    *res = region->end;
    expand_region(vseg, region, size);
    return 0;
    
}


void
vm_segment_free(vm_segment_t *vseg, vm_addr_t vaddr, vm_size_t size)
{
    vm_region_t *region = list_find(&vseg->regions, is_containing_addr, vaddr);
    KASSERT(region != NULL);
    // patrzymy czy dany region skróciæ, czy podzieliæ na dwa mniejsze
    if (region->begin == vaddr) {
        if (region->size == size) {
            list_remove(&vseg->regions, region);
            vm_lpool_free(&vm_unused_regions, region);
        } else {
            region->begin += size;
            region->size  -= size;
        }
    } else
    if (region->end == vaddr+size) {
        region->end -= size;
        region->size -= size;
    } else {
        vm_region_t *newreg = vm_lpool_alloc(&vm_unused_regions);
        newreg->begin = vaddr+size;
        newreg->end = region->end;
        newreg->size = region->end - newreg->begin;
        region->end = vaddr;
        region->size = region->end - region->begin;
        list_insert_in_order(&vseg->regions, newreg, is_prev);
    }
    vm_pmap_erase(&vseg->space->pmap, vaddr, size);
}


void
expand_region(vm_segment_t *segment, vm_region_t *region, vm_size_t size)
{
    vm_region_t *nextreg = list_next(&segment->regions, region);
    vm_size_t newaddr = region->end;
    region->end += size;
    region->size += size;
    if (nextreg == NULL) {
        // rozszerzamy tak¿e segment
        segment->end = region->end;
        segment->size = region->size;
    } else
    if (region->end == nextreg->begin) {
        // zlepiamy ze sob± regiony, miêdzy którymi nie ma ju¿ dziury
        region->end = nextreg->end;
        region->size += nextreg->size;
        list_remove(&segment->regions, nextreg); 
    }
    vm_pmap_fill(&segment->space->pmap, newaddr, size);
}

bool
is_prev(const vm_region_t *regA, const vm_region_t *regB)
{
    return (regA->begin < regB->begin);
}


bool
has_hole_after_reg(const vm_region_t *reg, vm_size_t size)
{
    vm_region_t *regnext = list_next(&reg->segment->regions, reg);
    // je¿eli to ostatni region.. to mo¿emy go rozszerzyæ spokojnie.
    if (regnext == NULL) return TRUE;
    // je¿eli nie to sprawdzamy czy zanim jest dziura o odpowiedniej wielko¶ci.
    return (reg->end + size <= regnext->begin);
}

bool
is_containing_addr(const vm_region_t *reg, vm_addr_t addr)
{
    return (reg->begin <= addr && addr < reg->end);
}


void *
vm_kern_alloc(vm_size_t size)
{
    void *res;
    vm_segment_alloc(&vm_kspace.seg_data, size, &res);
    return res;
}


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
static int expand_region(vm_segment_t *, vm_region_t *, vm_size_t, int mode);
static int do_first_region(vm_segment_t *c, vm_region_t **reg);
static bool is_containing_addr(const vm_region_t *reg, vm_addr_t addr);
int segment_grow(vm_segment_t *vseg, vm_size_t size);
int segment_descend(vm_segment_t *vseg, vm_size_t size);

enum {
    EXPAND_UP,
    EXPAND_DOWN
};

#define has_space(seg,size) (((seg)->size + (size)) <= (seg)->limit)

/**
 * Tworzy opis segmentu
 * @param vseg wska¼nik do deskryptora segmentu.
 * @param vsp wska¼nik do przestrzeni adresowej, w której znajduje siê seg.
 * @param base adres pierwszej strony w segmencie.
 * @param size rozmiar.
 * @param limit ograniczenie górne na rozmiar segmentu.
 * @param flags dodatkowe opcje.
 */
void
vm_segment_create(vm_segment_t *vseg, vm_space_t *vsp,
    vm_addr_t base, vm_size_t size, vm_size_t limit, int flags)
{
    vseg->space = vsp;
    vseg->base = base;
    vseg->size = size;
    vseg->end = vseg->base + vseg->size;
    vseg->limit = limit;
    vseg->flags = flags;
    if (flags & VM_SEGMENT_EXPDOWN) {
        vseg->search_func = has_hole_after_reg;
    } else {
        vseg->search_func = has_hole_after_reg;
    }
    list_create(&vseg->regions, offsetof(vm_region_t,L_regions), FALSE);
}

/**
 * Przydziela ci±g³y obszar w danym segmencie.
 * @param vseg deskryptor segmentu.
 * @param size rozmiar ci±g³ego obszaru.
 * @param _res adres zmiennej, do uzupe³nienia przydzielonym adresem.
 */

int
vm_segment_alloc(vm_segment_t *vseg, vm_size_t size, void *_res)
{
    int expand = EXPAND_UP;
    vm_addr_t *res = _res;
    size = PAGE_ROUND(size);
    vm_region_t *region = list_head(&vseg->regions);
    if (region == NULL) {
        if (do_first_region(vseg, &region)) return -1;
    }
    // sprawdzamy czy istnieje dziura pomiêdzy pocz±tkiem segmentu
    // a pierwszym regionem
    if (size < region->begin - vseg->base) {
        return expand_region(vseg, region, size, EXPAND_DOWN);
    }
    // Ok, no to szukamy dziury za regionem
    region = list_find(&vseg->regions, has_hole_after_reg, size);
    if (region == NULL) {
//         TRACE_IN("nie znalaz³em dziury");
        if (vseg->flags & VM_SEGMENT_EXPDOWN) {
            expand = EXPAND_DOWN;
            region = list_head(&vseg->regions);
            *res = region->begin - size;
        } else {
            region = list_tail(&vseg->regions);
            *res = region->end;
        }
    } else {
//         TRACE_IN("Znalazlem dziure!");
//         kprintf("seg: %p-%p (+%p)\n", vseg->base,vseg->end,vseg->size);
//         kprintf("reg: %p-%p (+%p)\n", region->begin,region->end, region->size);
        *res = region->end;
    }
    return expand_region(vseg, region, size, expand);
}


/**
 * Zwalnia ci±g³y obszar.
 * @param vseg deskryptor segmentu.
 * @param vaddr adres pierwszej strony w obszarze.
 * @param size rozmiar.
 */
void
vm_segment_free(vm_segment_t *vseg, vm_addr_t vaddr, vm_size_t size)
{
    vm_region_t *region = list_find(&vseg->regions, is_containing_addr, vaddr);
    KASSERT(region != NULL);
    // patrzymy, czy wyrzuciæ ca³y region, czy skróciæ go z pocz±tku.
    if (region->begin == vaddr) {
        if (region->size == size) {
            list_remove(&vseg->regions, region);
            vm_lpool_free(&vm_unused_regions, region);
        } else {
            region->begin += size;
            region->size  -= size;
        }
    } else
    // lub skróciæ od koñca.
    if (region->end == vaddr+size) {
        region->end -= size;
        region->size -= size;
    } else {
        // pozostaje tylko podzieliæ region
        vm_region_t *newreg = vm_lpool_alloc(&vm_unused_regions);
        newreg->begin = vaddr+size;
        newreg->end = region->end;
        newreg->size = region->end - newreg->begin;
        newreg->segment = vseg;
        region->end = vaddr;
        region->size = region->end - region->begin;
        list_insert_in_order(&vseg->regions, newreg, is_prev);
    }
    if (vseg->flags & VM_SEGMENT_NORMAL) {
        if (vseg->end == vaddr + size) {
            vseg->end -= size;
            vseg->size -= size;
        }
    } else  if (vseg->flags & VM_SEGMENT_EXPDOWN) {
        if (vseg->base == vaddr) {
            vseg->base += size;
            vseg->size -= size;
        }
    }
    vm_pmap_erase(&vseg->space->pmap, vaddr, size);
}

int
expand_region(vm_segment_t *seg, vm_region_t *region, vm_size_t size, int m)
{
    vm_addr_t newaddr;
    if (m == EXPAND_UP) {
//         TRACE_IN("rozszerzamy w gore");
        vm_region_t *nextreg = list_next(&seg->regions, region);
        newaddr = region->end;
        if (nextreg == NULL) {
            // rozszerzaj±c region rozszerzamy segment
            if (!has_space(seg,size)) return -1;
            seg->end += size;
            seg->size += size;
        }
        region->end += size;
        region->size += size;
        if (nextreg && region->end == nextreg->begin) {
            region->end += nextreg->size;
            region->size += nextreg->size;
            list_remove(&seg->regions, nextreg);
        }
    } else {
//         TRACE_IN("rozszerzamy w dol");
        vm_region_t *prevreg = list_prev(&seg->regions, region);
        if (prevreg == NULL) {
            if (!has_space(seg,size)) return -1;
            seg->base -= size;
            seg->size += size;
        }
        region->begin -= size;
        region->size += size;
        newaddr = region->begin;
        if (prevreg && prevreg->end == region->begin) {
            region->begin -= prevreg->size;
            region->size += prevreg->size;
            list_remove(&seg->regions, prevreg);
        }
    }
    vm_pmap_fill(&seg->space->pmap, newaddr, size,
        VM_PROT_RWX | VM_PROT_SYSTEM);
    return 0;
}


#if 0
int
expand_region(vm_segment_t *seg, vm_region_t *region, vm_size_t size, int m)
{
    vm_region_t *nextreg = list_next(&seg->regions, region);
    vm_region_t *prevreg = list_prev(&seg->regions, region);
    vm_size_t newaddr = region->end;
    if (m == EXPAND_UP && nextreg == NULL) {
        if (!has_space(seg,size)) return -1;
        // rozszerzamy tak¿e segment
        region->end += size;
        region->size += size;
        seg->end = region->end;
        seg->size += size;
    } else
    if (m == EXPAND_DOWN && prevreg == NULL) {
        if (!has_space(seg,size)) return -1;
        region->begin -= size;
        region->size += size;
        seg->base = region->begin;
        seg->size += size;
        newaddr = region->begin;
    } else {

    }
    if (region->end == nextreg->begin) {
        region->end += size;
        region->size += size;
        // zlepiamy ze sob± regiony, miêdzy którymi nie ma ju¿ dziury
        region->end = nextreg->end;
        region->size += nextreg->size;
        list_remove(&seg->regions, nextreg);
    }

    vm_pmap_fill(&seg->space->pmap, newaddr, size,
        VM_PROT_RWX | VM_PROT_SYSTEM);
    return 0;
}
#endif

bool
is_prev(const vm_region_t *regA, const vm_region_t *regB)
{
    return (regA->begin < regB->begin);
}


bool
has_hole_after_reg(const vm_region_t *reg, vm_size_t size)
{
    vm_region_t *regnext = list_next(&reg->segment->regions, reg);
    if (regnext == NULL) return FALSE;
    // je¿eli nie to sprawdzamy czy zanim jest dziura o odpowiedniej wielko¶ci.
    return (reg->end + size <= regnext->begin);
}


bool
is_containing_addr(const vm_region_t *reg, vm_addr_t addr)
{
    return (reg->begin <= addr && addr < reg->end);
}


int
do_first_region(vm_segment_t *vseg, vm_region_t **reg)
{
    vm_region_t *region = vm_lpool_alloc(&vm_unused_regions);
    region->segment = vseg;
    region->begin = vseg->base;
    region->size = vseg->size;
    region->end = vseg->end;
    vm_pmap_fill(&vseg->space->pmap, vseg->base, vseg->size,
        VM_PROT_RWX);
    list_insert_head(&vseg->regions, region);
    *reg = region;
    return 0;
}

void *
vm_kern_alloc(vm_size_t size)
{
    void *res;
    vm_segment_alloc(vm_kspace.seg_data, size, &res);
    return res;
}


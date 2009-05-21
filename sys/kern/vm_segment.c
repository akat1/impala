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
#include <sys/vm.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/string.h>
#include <sys/utils.h>

static bool has_hole_after_reg(const vm_region_t *reg, vm_size_t size);
static bool is_prev(const vm_region_t *regA, const vm_region_t *regB);
static int expand_region(vm_seg_t *, vm_region_t *, vm_size_t, int mode,
    void *na);
static int do_first_region(vm_seg_t *c, vm_region_t **reg);
static bool is_containing_addr(const vm_region_t *reg, vm_addr_t addr);
int segment_grow(vm_seg_t *vseg, vm_size_t size);
int segment_descend(vm_seg_t *vseg, vm_size_t size);

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
vm_seg_create(vm_seg_t *vseg, vm_space_t *vsp,
    vm_addr_t base, vm_size_t size, vm_size_t limit, vm_prot_t p, int flags)
{
    vseg->space = vsp;
    vseg->base = base;
    vseg->size = size;
    vseg->end = base + size;
    vseg->limit = limit;
    vseg->flags = flags;
    vseg->prot = p;
//     DEBUGF("segc %p-%p", base, vseg->end);
    if (flags & VM_SEG_EXPDOWN) {
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
vm_seg_alloc(vm_seg_t *vseg, vm_size_t size, void *_res)
{
    vm_addr_t na;
    if (vm_seg_reserve(vseg, size, &na)) return -1;
//     TRACE_IN("seg=%p size=%p fill=%p", vseg, size, na);
    vm_pmap_fill(&vseg->space->pmap, na, size, vseg->prot);
    vm_addr_t *res = _res;
    *res = na;
    return 0;
}

/**
 * Rezerwuje ci±g³y obszar w danym segmencie.
 * @param vseg deskryptor segmentu.
 * @param size rozmiar ci±g³ego obszaru.
 * @param _res adres zmiennej, do uzupe³nienia przydzielonym adresem.
 */
int
vm_seg_reserve(vm_seg_t *vseg, vm_size_t size, void *_res)
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
        *res = region->begin - size;
        return expand_region(vseg, region, size, EXPAND_DOWN, _res);
    }

    // Ok, no to szukamy dziury za regionem
    region = list_find(&vseg->regions, has_hole_after_reg, size);
    if (region == NULL) {
        if (vseg->flags & VM_SEG_EXPDOWN) {
            expand = EXPAND_DOWN;
            region = list_head(&vseg->regions);
            *res = region->begin - size;
        } else {
            region = list_tail(&vseg->regions);
            *res = region->end;
        }
    } else {
        *res = region->end;
    }
    return expand_region(vseg, region, size, expand, _res);
}


/**
 * Zwalnia ci±g³y obszar i jego strony.
 * @param vseg deskryptor segmentu.
 * @param vaddr adres pierwszej strony w obszarze.
 * @param size rozmiar.
 */
void
vm_seg_free(vm_seg_t *vseg, vm_addr_t vaddr, vm_size_t size)
{
    vm_seg_release(vseg, vaddr, size);
    vm_pmap_erase(&vseg->space->pmap, vaddr, size);
}

/**
 * Zwalnia ci±g³y obszar.
 * @param vseg deskryptor segmentu.
 * @param vaddr adres pierwszej strony w obszarze.
 * @param size rozmiar.
 */
void
vm_seg_release(vm_seg_t *vseg, vm_addr_t vaddr, vm_size_t size)
{
    bool deleteRegion = FALSE;
    vm_region_t *region = list_find(&vseg->regions, is_containing_addr, vaddr);
    KASSERT(region != NULL);
    // patrzymy, czy wyrzuciæ ca³y region, czy skróciæ go z pocz±tku.
    if (region->begin == vaddr) {
        if (region->size == size) {
            deleteRegion = TRUE;
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
    if (vseg->flags & VM_SEG_NORMAL) {
        if (vseg->end == vaddr + size) {
            vm_region_t *prevreg = list_prev(&vseg->regions, region);
            vseg->end -= size;
            vseg->size -= size;
            if (vaddr == region->begin && prevreg) {
                // usunelismy caly region, kasujmy wiec dziure przed nim
                vseg->end -= vaddr - prevreg->end;
                vseg->size -= vaddr - prevreg->end;
            }
        }
    } else  if (vseg->flags & VM_SEG_EXPDOWN) {
        if (vseg->base == vaddr) {
            vm_region_t *nextreg = list_next(&vseg->regions, region);
            vseg->base += size;
            vseg->size -= size;
            if (size == region->size && nextreg) {
                vseg->base = nextreg->begin;
                vseg->size -= (nextreg->begin - region->end);
            }
        }
    }
    if (deleteRegion) {
        list_remove(&vseg->regions, region);
        vm_lpool_free(&vm_unused_regions, region);
    }
}


/**
 * Tworzy kopiê segmentu.
 * @param vseg deskryptor segmentu.
 * @param vaddr adres pierwszej strony w obszarze.
 * @param size rozmiar.
 */
int
vm_seg_clone(vm_seg_t *dst, vm_space_t *space, vm_seg_t *src)
{
    dst->space = space;
    dst->prot = src->prot;
    dst->base = src->base;
    dst->size = src->size;
    dst->end = src->end;
    vm_region_t *reg = NULL;
    vm_region_t *clonereg;
    while ( (reg = list_next(&src->regions, reg)) ) {
        clonereg = vm_lpool_alloc(&vm_unused_regions);
        clonereg->begin = reg->begin;
        clonereg->size = reg->size;
        clonereg->end = reg->end;
        list_insert_tail(&dst->regions, clonereg);
        vm_pmap_fill(&space->pmap, clonereg->begin, clonereg->size,
            dst->prot);
        vm_addr_t SRC,DST;
        vm_segmap(dst, clonereg->begin, clonereg->size, &DST);
        vm_segmap(src, reg->begin, reg->size, &SRC);
        mem_cpy((void*)DST, (void*)SRC, reg->size);
        vm_unmap(DST, reg->size);
        vm_unmap(SRC, reg->size);
    }
    return 0;
}


/**
 * Odwzorowywuje fragment innego segmentu
 * @param dst deskryptor segmentu docelowego
 * @param src deskryptor segmentu ¼ród³owego
 * @param addr adres w segmencie ¼ród³owym
 * @param size rozmiar
 * @param res
 *
 * Przydzielony obszar mo¿na zwalniac u¿ywaj±c vm_seg_free().
 */

int
vm_seg_map(vm_seg_t *dst, const vm_seg_t *src, vm_addr_t addr, vm_size_t size,
    void *res)
{
    vm_addr_t dstaddr;
    if (vm_seg_reserve(dst, size, &dstaddr)) return -1;
    vm_pmap_map(&dst->space->pmap, dstaddr, &src->space->pmap, addr, size);
    return 0;
}


/**
 * Zmienia ochronê segmentu.
 * @param seg deskryptor segmentu.
 * @param newprot nowe prawa dostêpu.
 *
 * Procedura zmienia prawa dostêpu do ka¿dej strony przydzielonej do
 * danego segmentu.
 */
void
vm_seg_protect(vm_seg_t *seg, vm_prot_t newprot)
{
    seg->prot = newprot;
    vm_region_t *reg = NULL;
    while ( (reg = list_next(&seg->regions, reg)) ) {
        vm_pmap_fillprot(&seg->space->pmap, reg->begin, reg->size, newprot);
    }
}


void
vm_seg_destroy(vm_seg_t *seg)
{
    vm_region_t *reg = NULL;
    vm_region_t *next;
    while ( reg ) {
        vm_seg_free(seg, reg->begin, reg->size);
        next = list_next(&seg->regions, reg);
        list_remove(&seg->regions, reg);
        vm_lpool_free(&vm_unused_regions, reg);
        reg = next;
    }
}

int
expand_region(vm_seg_t *seg, vm_region_t *region, vm_size_t size, int m,
    void *_newaddr)
{

    vm_addr_t newaddr;
    if (m == EXPAND_UP) {
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
    vm_addr_t *na = _newaddr;
    *na = newaddr;
    return 0;
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
do_first_region(vm_seg_t *vseg, vm_region_t **reg)
{
    vm_region_t *region = vm_lpool_alloc(&vm_unused_regions);
    region->segment = vseg;
    region->begin = vseg->base;
    region->size = vseg->size;
    region->end = vseg->end;
    list_insert_head(&vseg->regions, region);
    *reg = region;
    return 0;
}


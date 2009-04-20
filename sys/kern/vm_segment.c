/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/vm.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/kprintf.h>
#include <sys/utils.h>

static vm_addr_t _vm_segment_expand(vm_segment_t *vseg, vm_size_t s);
static void _region_expand(vm_segment_t *vseg, vm_region_t *r,
    vm_region_t *rnext, vm_size_t howmuch);
static vm_region_t *_find_region(vm_segment_t *vseg, vm_addr_t a);
static bool _is_first(const vm_region_t *regA, const vm_region_t *regB);

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

vm_addr_t
vm_segment_alloc(vm_segment_t *vseg, vm_size_t size)
{
//     TRACE_IN("vseg=%p size=%u", vseg, size);
    vm_region_t *r = NULL;
    vm_region_t *rnext = NULL;
    size = PAGE_ROUND(size)*PAGE_SIZE;
    // Przeszukujemy regiony w segmencie.
    while ((r = list_next(&vseg->regions, r))) {
        rnext = list_next(&vseg->regions, r);
        if (rnext && rnext->begin - (r->begin + r->size)) {
            vm_addr_t addr = r->begin + r->size;
            _region_expand(vseg, r, rnext, size);
//             TRACE_IN("+returning %p", addr);
            return addr;
        }
        rnext = r;
    }
    if (rnext) {
//         TRACE_IN("Expanding...");
        vm_addr_t va = rnext->begin + rnext->size;
        _region_expand(vseg, rnext, NULL, size);
//         TRACE_IN("returning %p", va);
        return va;
    } else {
        panic("Po prostu wielkie dzieki!");
    }
    return 0;
}


void
vm_segment_free(vm_segment_t *vseg, vm_addr_t vaddr, vm_size_t size)
{
//     TRACE_IN("vseg=%p vaddr=%p size=%u", vseg, vaddr, size);
    vm_region_t *rfirst = list_head(&vseg->regions);
    vm_region_t *r = _find_region(vseg, vaddr);
    KASSERT(r != NULL);
    KASSERT(_find_region(vseg, vaddr + size - 1) != NULL);
    // Chcê aby na razie w segmencie by³ zawsze jakis region, wiec jezeli ten region to pierwszy to go nie kasujemy.
    if (rfirst != r && r->begin == vaddr && r->begin + r->size == vaddr+size) {
//         TRACE_IN("Deleting whole region");
        list_remove(&vseg->regions, r);
        vm_lpool_free(&vm_unused_regions, r);
    } else
    if (r->begin != vaddr && r->begin + r->size == vaddr+size) {
        r->size -= size;
        r->end -= size;
    } else
    if (r->begin == vaddr && r->size != size) {
        r->begin += size;
    } else {
        vm_region_t *rnew = vm_lpool_alloc(&vm_unused_regions);
        KASSERT(rnew != NULL);
        rnew->begin = vaddr+size;
        rnew->size = (r->end - (vaddr+size));
        rnew->end = rnew->begin + rnew->size;
        r->size = (vaddr - r->begin);
        r->end = r->begin + r->size;
        list_insert_in_order(&vseg->regions, rnew, (list_less_f*)_is_first);
    }
}

bool
_is_first(const vm_region_t *regA, const vm_region_t *regB)
{
    return (regA->begin < regB->begin);
}

vm_region_t *
_find_region(vm_segment_t *vms, vm_addr_t a)
{
    vm_region_t *r = NULL;
    while ( (r = list_next(&vms->regions, r)) ) {
        if (r->begin <= a &&  a < (r->begin+r->size) ) {
            return r;
        }
    }
    return NULL;
}

vm_addr_t
_vm_segment_expand(vm_segment_t *vms, vm_size_t size)
{
    int pages = PAGE_ROUND(size);
    size = pages*PAGE_SIZE;
    if (vms->size + size < vms->limit) {
        vm_addr_t b = vms->base + vms->size;
        for (int i = 0; i < pages; i++) {
            vm_page_t *pg = vm_alloc_page();
            if (vms->space == &vm_kspace) {
                pg->kvirt_addr = vms->base + vms->size;
            }
            vm_pmap_insert(&vms->space->pmap, pg, vms->base + vms->size);
            vms->size += PAGE_SIZE;
            vms->end += PAGE_SIZE;
        }
        return b; 
    } else {
        return (0-1); // smieszny triczek na unsigned
    }
}

void
_region_expand(vm_segment_t *vseg, vm_region_t *r,
    vm_region_t *rnext, vm_size_t howmuch)
{
    if (rnext) {
        // Nie jestesmy na koncu segmentu, wiec rozszerzamy jedynie region.
        vm_addr_t va = r->begin + r->size;
        for (int i = 0; i < howmuch; i+=PAGE_SIZE) {
            vm_page_t *pg = vm_alloc_page();
            vm_pmap_insert(&vseg->space->pmap, pg, va);
            va += PAGE_SIZE;
        }
        r->size += howmuch;
        // TODO: laczenie regionow.
    } else {
        // Jestesmy na koncu 
        r->size += howmuch;
        r->end += howmuch;
        _vm_segment_expand(vseg, howmuch);
    }
}




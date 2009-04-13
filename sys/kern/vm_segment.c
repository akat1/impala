#include <sys/types.h>
#include <sys/vm.h>
#include <sys/kprintf.h>
#include <sys/utils.h>

static vm_addr_t _vm_segment_expand(vm_segment_t *vseg, vm_size_t s);
static void _region_expand(vm_segment_t *vseg, vm_region_t *r,
    vm_region_t *rnext, vm_size_t howmuch);

void
vm_segment_create(vm_segment_t *vseg, vm_space_t *vsp,
    vm_addr_t base, vm_size_t size, vm_size_t limit)
{
    vseg->space = vsp;
    vseg->base = base;
    vseg->size = size;
    vseg->limit = limit;
    list_create(&vseg->regions, offsetof(vm_region_t,L_regions), FALSE);
}

vm_addr_t
vm_segment_alloc(vm_segment_t *vseg, vm_size_t size)
{
    TRACE_IN("vseg=%p size=%u", vseg, size);
    vm_region_t *r = NULL;
    vm_region_t *rnext = NULL;
    size = PAGE_ROUND(size)*PAGE_SIZE;
    // Przeszukujemy regiony w segmencie.
    while ((r = list_next(&vseg->regions, r))) {
        rnext = list_next(&vseg->regions, r);
        if (rnext && rnext->begin - (r->begin + r->size)) {
            vm_addr_t addr = r->begin + r->size;
            _region_expand(vseg, r, rnext, size);
            return addr;
        }
        rnext = r;
    }
    if (rnext) {
        TRACE_IN("Expanding...");
        vm_addr_t va = rnext->begin + rnext->size;
        _region_expand(vseg, rnext, NULL, size);
        return va;
    } else {
        panic("Po prostu wielkie dzieki!");
    }
    return 0;
}


vm_addr_t
_vm_segment_expand(vm_segment_t *vms, vm_size_t size)
{
    TRACE_IN("vms=%p size=%u", vms, size);
    int pages = PAGE_ROUND(size);
    size = pages*PAGE_SIZE;
    if (vms->size + size < vms->limit) {
        TRACE_IN("found");
        vm_addr_t b = vms->base + vms->size;
        for (int i = 0; i < pages; i++) {
            vm_page_t *pg = vm_alloc_page();
            if (vms->space == &vm_kspace) {
                pg->kvirt_addr = vms->base + vms->size;
            }
            TRACE_IN("inserting in pmap %p %p", pg->phys_addr, pg->kvirt_addr);
            vm_pmap_insert(&vms->space->pmap, pg, vms->base + vms->size);
            vms->size += PAGE_SIZE;
        }
        return b; 
    } else {
        TRACE_IN("not found");
        return (0-1); // smieszny triczek na unsigned
    }
}

void
_region_expand(vm_segment_t *vseg, vm_region_t *r,
    vm_region_t *rnext, vm_size_t howmuch)
{
    TRACE_IN("vseg=%p r=%p rnext=%p howmuch=%u", vseg, r, rnext,
        howmuch);
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
        _vm_segment_expand(vseg, howmuch);
    }
    TRACE_IN("expanded");
}



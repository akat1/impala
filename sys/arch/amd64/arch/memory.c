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
#include <sys/vm.h>
#include <sys/vm/vm_pmap.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/utils.h>
#include <sys/string.h>
#include <sys/thread.h>
#include <machine/interrupt.h>
#include <machine/memory.h>
#include <machine/cpu.h>
#include <machine/io.h>

size_t vm_physmem_max;
size_t vm_physmem_free;


static vm_prot_t _pmap_page_prot(const vm_pmap_t *pmap, vm_addr_t addr);
static vm_page_t * pmap_get_page(const vm_pmap_t *pmap, vm_addr_t addr);
static bool vm_pmap_init0(vm_pmap_t *vpm);
static vm_ptable_t *_pmap_get_pt(vm_pmap_t *vpm, vm_addr_t addr, bool create);
static vm_ptable_t *_pmap_find_pt(const vm_pmap_t *vpm, vm_addr_t addr);
static bool _ptable_is_empty(vm_ptable_t *pt);

static vm_ptable_t *_alloc_ptable(vm_page_t **pg);
static vm_page_t * _alloc_kpage(void);
static vm_page_t * _alloc_page(void);

/// Wskaźnik do tablicy opisu stron.
static vm_page_t *vm_pages;
/// Ilosc stron przeznaczona na opis.

static vm_seg_t kseg_text;
static vm_seg_t kseg_data;
static vm_seg_t kseg_stack;

static vm_addr_t kbrk;
static vm_addr_t kbootstrap;
static vm_addr_t kstart;
static vm_addr_t kend;
static vm_addr_t ktextend;

static mutex_t mem_mtx;

extern int kernel_start;
extern int kernel_text_end;
extern int kernel_end;
extern int KERNEL_BOOTSTRAP;
extern int KERNEL_START;
#define _GLOBAL(a,f) (VM_SPACE_KERNEL <= a && a < VM_SPACE_KERNEL_E)? f : 0
static void tlb_flush(vm_addr_t );


void
tlb_flush(vm_addr_t va)
{
    __asm__ volatile(
        "invlpg (%0)"
        :
        : "r" (va)
        : "memory");
}

/*========================================================================
 * Stronicowanie,
 */


///
void
vm_pmap_switch(const vm_pmap_t *pm)
{
    cpu_set_cr3(pm->physdir);
}

/// Włącza stronicowanie.
void
vm_enable_paging()
{
    cpu_set_cr0(cpu_get_cr0() | CR0_PG);
}

void
vm_enable_cache()
{
    uint64_t cr0 = cpu_get_cr0();
    uint64_t cr4 = cpu_get_cr4();
    UNSET(cr0, CR0_CD|CR0_NW);
    SET(cr4, CR4_PGE);
    cpu_set_cr0(cr0);
    cpu_set_cr4(cr4);

}

void
vm_disable_cache()
{
    uint64_t cr0 = cpu_get_cr0();
    uint64_t cr4 = cpu_get_cr4();
    SET(cr0, CR0_CD|CR0_NW);
    UNSET(cr4, CR4_PGE);
    cpu_set_cr0(cr0);
    cpu_set_cr4(cr4);
}

/// Wyłącza stronicowanie.
void
vm_disable_paging()
{
    cpu_set_cr0(cpu_get_cr0() & ~CR0_PG);
}

#define PAUSE() for (int i = 0; i < 99999999; i++);
#define PAUSE2() for (int i = 0; i < 99999999; i++);

/*========================================================================
 * Inicjalizacja.
 */

static size_t _avail_physmem(void);
static void create_kernel_space(void);
static void initialize_internal(void);
static void _collect_pages(void);

void
vm_low_init()
{
    extern int kernel_end;
    kbrk = kend = PAGE_ROUND((uintptr_t)&kernel_end);
    ktextend = PAGE_ROUND((uintptr_t)&kernel_text_end);
    kstart = (uintptr_t)&KERNEL_START;
    kbootstrap = (uintptr_t)&KERNEL_BOOTSTRAP;

    mutex_init(&mem_mtx, MUTEX_NORMAL);
    
    vm_physmem_max = _avail_physmem();
    _collect_pages();
    create_kernel_space();
    initialize_internal();
    vm_enable_cache();
    
}

vm_page_t *page;

#define mem_lock() int xxx=spltty()
//MUTEX_LOCK(&mem_mtx, "mem")
#define mem_unlock() splx(xxx);
//mutex_unlock(&mem_mtx)

/// Inicjalizuje przestrzeń adresową jądra.
void
create_kernel_space()
{
    // ustaw segmenty.
    vm_kspace.seg_text = &kseg_text;
    vm_kspace.seg_data = &kseg_data;
    vm_kspace.seg_stack = &kseg_stack;
    vm_kspace.mtx = NULL;
    vm_seg_create(vm_kspace.seg_text, &vm_kspace, kstart,
        ktextend-kstart, ktextend-kstart, VM_PROT_RWX | VM_PROT_SYSTEM,
        VM_SEG_NORMAL);
    vm_seg_create(vm_kspace.seg_data, &vm_kspace, ktextend,
        kbrk-ktextend, VM_SPACE_DATA_LIMIT-ktextend+kstart,
        VM_PROT_RWX | VM_PROT_SYSTEM, VM_SEG_NORMAL);
    vm_seg_create(vm_kspace.seg_stack, &vm_kspace,
        VM_SPACE_KERNEL_E, 0, 0, VM_PROT_RWX | VM_PROT_SYSTEM,
        VM_SEG_EXPDOWN);


    // stwórz odwzorowywanie jądra.
    vm_pmap_t *kmap = &vm_kspace.pmap;
    vm_pmap_init0(kmap);


    // tworzymy tablice stron dla docelowego obszaru jądra
    vm_addr_t vaddr = kstart;
    for (int i = 0; i < VM_SPACE_KERNEL_PTABLES; i++) {
        _pmap_get_pt(kmap, vaddr, TRUE);
        vaddr += PAGE_SIZE * PTABLE_ENTRIES;
    }

    vm_addr_t paddr = kbootstrap;
    for (vaddr = kstart; vaddr < kseg_data.end ; vaddr += PAGE_SIZE) {
        int i = PAGE_NUM(paddr);
        vm_pmap_insert(kmap, &vm_pages[i], vaddr,
            VM_PROT_RWX|VM_PROT_SYSTEM);
        paddr += PAGE_SIZE;
    }

    vm_pmap_switch(kmap);
}


void
initialize_internal()
{
    vm_seg_t *kdata = vm_kspace.seg_data;

    // przydzielamy miejsce na poczatkowe regiony
    vm_page_t *page = _alloc_page();
    vm_lpool_create_(&vm_unused_regions, offsetof(vm_region_t,L_regions),
            sizeof(vm_region_t), VM_LPOOL_NORMAL, (void*)page->kvirt_addr);
    page = _alloc_page();
    vm_lpool_insert_empty(&vm_unused_regions, (void*)page->kvirt_addr);


    vm_region_t *reg = vm_lpool_alloc(&vm_unused_regions);
    reg->begin = kdata->base;
    reg->size = kdata->size;
    reg->end = reg->begin + reg->size;
    reg->segment = kdata;
    list_insert_head(&kdata->regions, reg);
    cpu_set_cr0(cpu_get_cr0() | CR0_WP); //włączamy ochronę przed zapisem
}

/// Wykrywa ilość zainstalowanej pamięci RAM.
size_t
_avail_physmem()
{
    size_t physmem;
    enum {
        CMOS_PORT_OUT = 0x70,
        CMOS_PORT_IN = 0x71,
        CMOS_REQ_HIMEM = 0x31,
        CMOS_REQ_LOMEM = 0x30
    };
    io_out8(CMOS_PORT_OUT, CMOS_REQ_LOMEM);
    physmem = io_in8(CMOS_PORT_IN);
    io_out8(CMOS_PORT_OUT, CMOS_REQ_HIMEM);
    physmem |= (io_in8(CMOS_PORT_IN) << 8);
    return physmem >> 2;
}

void
_collect_pages()
{
    vm_pages = (vm_page_t*)kbrk;
    kbrk += PAGE_ROUND(vm_physmem_max*sizeof(vm_page_t));

    vm_addr_t _kernel_start = kbootstrap;
    vm_addr_t _kernel_end = kbootstrap + kbrk - (uintptr_t)&KERNEL_START;

    vm_paddr_t physaddr = 0;
    list_create(&vm_free_pages, offsetof(vm_page_t, L_pages), FALSE);

    for (int i = 0; i < vm_physmem_max; i++, physaddr += PAGE_SIZE) {
            vm_pages[i].refcnt = 0;
            vm_pages[i].kvirt_addr = 0;
            vm_pages[i].phys_addr = physaddr;
            if (_kernel_start <= physaddr && physaddr < _kernel_end) {
                vm_pages[i].kvirt_addr = (uintptr_t)&KERNEL_START + physaddr
                    - kbootstrap;
            }
            if (_kernel_end <= physaddr) {
                vm_pages[i].flags = PAGE_FREE;
                list_insert_tail(&vm_free_pages, &vm_pages[i]);
            }
    }
    vm_physmem_free = list_length(&vm_free_pages);
}


/*========================================================================
 * Obsługa vm_pmap_t
 */

/**
 * Inicjalizuje odwzorowanie stron.
 * @param vpm odwzorowanie stron.
 * @return FALSE wtedy i tylko wtedy, gdy nie można było przydzielić
 *         strony na nowy katalog stron.
 */

bool
vm_pmap_init(vm_pmap_t *vpm)
{
    vm_page_t *page;
    memzero(vpm, sizeof(*vpm));
    vpm->pdir = (vm_ptable_t*) _alloc_ptable(&page);
    vpm->physdir = page->phys_addr;
    memzero(vpm->pdir, PAGE_SIZE);
    memcpy(vpm->pdir, vm_kspace.pmap.pdir, PAGE_SIZE);
    return (vpm->pdir != 0);
}

bool
vm_pmap_init0(vm_pmap_t *vpm)
{
    memzero(vpm, sizeof(*vpm));
    vm_page_t *page = _alloc_kpage();
    vpm->physdir = page->phys_addr;
    vpm->pdir = (vm_ptable_t*) page->kvirt_addr;
    memzero(vpm->pdir, PAGE_SIZE);
    return TRUE;
}


/**
 * Ustawia odwzorowywanie strony.
 * @param vpm odwzorowanie stron.
 * @param p strona.
 * @param va wirtualny adres strony.
 * @param prot poziom ochrony strony.
 * @return FALSE wtedy i tylko wtedy gdy nie można było przydzielić pamięci
 *         na nową tablicę stron.
 */
bool
vm_pmap_insert(vm_pmap_t *vpm, vm_page_t *p, vm_addr_t va, vm_prot_t prot)
{
    bool _G = _GLOBAL(va, PTE_G);
    vm_addr_t pa = p->phys_addr;
    int pte = PAGE_TBL(va);
    mem_lock();
    vm_ptable_t *pt = _pmap_get_pt(vpm, va, TRUE);
    tlb_flush(va);
    if (pt == NULL) {
        mem_unlock();
        return FALSE;
    }
    p->refcnt++;
    pt->table[pte] = PTE_ADDR(pa) | PTE_PRESENT
        | PROT_TO_PTEFLAGS(prot)
        | _G;
    mem_unlock();
    return TRUE;
}

/**
 * Ustawia odwzorowywanie strony.
 * @param vpm odwzorowanie stron.
 * @param pa adres fizyczny strony.
 * @param va wirtualny adres strony.
 * @param prot poziom ochrony strony.
 * @return FALSE wtedy i tylko wtedy gdy można było przydzielić pamięci
 *         na nową tablicę stron.
 *
 * Procedura jest wykorzystywana przez swój odpowiednik dla vm_page_t.
 * Istnieje ponieważ nie dla wszystkich stron robimy opis. Np
 * Nie będziemy zarządzań stronami z kodem jądra.
 */
bool
vm_pmap_insert_(vm_pmap_t *vpm, vm_paddr_t pa, vm_addr_t va, vm_prot_t prot)
{
    return vm_pmap_insert(vpm, &vm_pages[PAGE_NUM(pa)], va, prot);
}

bool
vm_pmap_fill(vm_pmap_t *pmap, vm_addr_t addr, vm_size_t size, vm_prot_t prot)
{
    for (size+=addr; addr < size; addr += PAGE_SIZE) {
        vm_page_t *p = vm_alloc_page();
        if(!vm_pmap_insert(pmap, p, addr, prot))
            return FALSE;
    }
    return TRUE;
}

/**
 * Ustawia prawa dostępu do istniejących stron.
 * @param pmap odwzorowanie stron które modyfikujemy
 * @param addr początek fragmentu pamięci, któremu ustawiamy prawa dostępu
 * @param size rozmiar fragmentu pamięci, ktremu ustawiamy prawa dostępu
 * @param prot żądane prawa dostępu do stron
 */

void
vm_pmap_fillprot(vm_pmap_t *pmap, vm_addr_t addr, vm_size_t size,
                  vm_prot_t prot)
{
    for (size+=addr; addr < size; addr += PAGE_SIZE) {
        int pte = PAGE_TBL(addr);
        vm_ptable_t *pt = _pmap_find_pt(pmap, addr);
        if (pt == NULL)
            continue;
        pt->table[pte] = (pt->table[pte] & ~(PTE_RW|PTE_US))
                          | PROT_TO_PTEFLAGS(prot);
    }
}

void
vm_pmap_physmap(vm_pmap_t *pmap, vm_addr_t dst_addr, vm_paddr_t src_addr,
                      vm_size_t size, vm_prot_t prot)
{
    int n = PAGE_NUM(src_addr);

    for (size+=dst_addr; dst_addr < size; dst_addr += PAGE_SIZE, n++) {
        vm_page_t *page = &vm_pages[n];
        vm_pmap_insert(pmap, page, dst_addr, prot);
    }
}


void
vm_pmap_map(vm_pmap_t *dst_pmap, vm_addr_t dst_addr, const vm_pmap_t *src_pmap,
    vm_addr_t src_addr, vm_size_t size)
{
    for (size+=dst_addr; dst_addr < size; dst_addr += PAGE_SIZE) {
        vm_page_t *page = pmap_get_page(src_pmap, src_addr);
        int prot = _pmap_page_prot(src_pmap, src_addr);
        vm_pmap_insert(dst_pmap, page, dst_addr, prot);
        src_addr += PAGE_SIZE;
    }
}

void
vm_pmap_clone(vm_pmap_t *dst, const vm_pmap_t *src)
{
    vm_pmap_init(dst);
    int user_l4_limit = PAGE_PML4(VM_SPACE_KERNEL);
    for (int l4 = 0; l4 < user_l4_limit; l4++) {
        if (!(src->pdir->table[l4] & PDE_PRESENT))
            continue;
        vm_ptable_t *src_pdpt = (vm_ptable_t *)vm_ptov(PTE_ADDR(src->pdir->table[l4]));
        for (int l3 = 0; l3 < PTABLE_ENTRIES; l3++) {
            if (!(src_pdpt->table[l3] & PDE_PRESENT))
                continue;
            vm_ptable_t *src_pd = (vm_ptable_t *)vm_ptov(PTE_ADDR(src_pdpt->table[l3]));
            for (int l2 = 0; l2 < PTABLE_ENTRIES; l2++) {
                if (!(src_pd->table[l2] & PDE_PRESENT))
                    continue;
                vm_ptable_t *src_pt = (vm_ptable_t *)vm_ptov(PTE_ADDR(src_pd->table[l2]));
                vm_addr_t base = ((vm_addr_t)l4 << 39) | ((vm_addr_t)l3 << 30) | ((vm_addr_t)l2 << 21);
                for (int l1 = 0; l1 < PTABLE_ENTRIES; l1++) {
                    if (!(src_pt->table[l1] & PTE_PRESENT))
                        continue;
                    vm_addr_t va = base | ((vm_addr_t)l1 << PAGE_SHIFT);
                    vm_paddr_t pa = PTE_ADDR(src_pt->table[l1]);
                    vm_prot_t prot = _pmap_page_prot(src, va);
                    vm_page_t *pg = &vm_pages[PAGE_NUM(pa)];
                    vm_pmap_insert(dst, pg, va, prot);
                }
            }
        }
    }
}

/**
 * Zwalnia stronę.
 */
bool
vm_pmap_remove(vm_pmap_t *pmap, vm_addr_t va)
{
    int pte = PAGE_TBL(va);
    mem_lock();
    vm_ptable_t *pt = _pmap_find_pt(pmap, va);
    if (pt == NULL) {
        mem_unlock();
        return FALSE;
    }
    if (pt->table[pte] & PTE_PRESENT) {
        pt->table[pte] = PTE_ADDR(pt->table[pte]);
        vm_page_t *pg = &vm_pages[PAGE_NUM(pt->table[pte])];
        KASSERT(pg != NULL);
        KASSERT(pg->refcnt>0);
        pg->refcnt--;
        if (pg->refcnt == 0) {
            list_insert_tail(&vm_free_pages, pg);
        }
    }
    pt->table[pte] = 0;
    if (_ptable_is_empty(pt)) {
        int pml4i = PAGE_PML4(va);
        int pdpti = PAGE_PDPT(va);
        int pdi = PAGE_PDIR(va);
        vm_ptable_t *pdpt = (vm_ptable_t*)vm_ptov(PTE_ADDR(pmap->pdir->table[pml4i]));
        vm_ptable_t *pd = (vm_ptable_t*)vm_ptov(PTE_ADDR(pdpt->table[pdpti]));
        pd->table[pdi] = 0;
        vm_seg_free(vm_kspace.seg_data, (vm_addr_t)pt, PAGE_SIZE);
    }
    tlb_flush(va);
    mem_unlock();
    return TRUE;
}


bool
vm_pmap_erase(vm_pmap_t *pmap, vm_addr_t addr, vm_size_t size)
{
    for (size += addr; addr < size; addr += PAGE_SIZE) {
        if(vm_pmap_remove(pmap, addr)==FALSE)
            return FALSE;
    }
    return TRUE;
}



/**
 * Tłumaczy adres wirtualny na fizyczny.
 * @param vpm odwzorowanie stron.
 * @param va adres wirtualny.
 * @return adres fizyczny
 * @warning procedura jest zdefiniowiowana jedynie dla istniejących
 *          w danym odwzorowaniu adresów wirtualnych.
 */

vm_paddr_t
vm_pmap_phys(const vm_pmap_t *vpm, vm_addr_t va)
{
    int pte = PAGE_TBL(va);
    uintptr_t off = PAGE_OFF(va);
    vm_ptable_t *pt = _pmap_find_pt(vpm, va);
    return PTE_ADDR(pt->table[pte]) + off;
}


/**
 * Sprawdza czy dany wirtualny adres istnieje.
 * @param vpm odwzorowanie stron.
 * @param va adres wirtualny.
 * @return TRUE wtedy i tylko wtedy, gdy adres istniej.
 */
bool
vm_pmap_is_avail(const vm_pmap_t *vpm, vm_addr_t va)
{
    int pte = PAGE_TBL(va);
    vm_ptable_t *pt = _pmap_find_pt(vpm, va);
    if (!pt) return FALSE;
    return pt->table[pte] & PTE_PRESENT;
}

/**
 * Zwraca flagi ustawione dla strony związanej z danym adresem wirtualnym
 */

vm_prot_t
_pmap_page_prot(const vm_pmap_t *pmap, vm_addr_t addr)
{
    int pte = PAGE_TBL(addr);
    vm_ptable_t *pt = _pmap_find_pt(pmap, addr);
    int flags = PTE_FLAGS(pt->table[pte]);
    return PTEFLAGS_TO_PROT(flags);
}


static vm_ptable_t *
_pmap_get_pt(vm_pmap_t *vpm, vm_addr_t addr, bool create)
{
    uintptr_t flags = PDE_PRESENT | PDE_RW;
    if (!_GLOBAL(addr, TRUE))
        flags |= PDE_US;

    int pml4i = PAGE_PML4(addr);
    int pdpti = PAGE_PDPT(addr);
    int pdi = PAGE_PDIR(addr);

    vm_ptable_t *pml4 = vpm->pdir;
    if (!(pml4->table[pml4i] & PDE_PRESENT)) {
        if (!create) return NULL;
        vm_page_t *pg;
        vm_ptable_t *pdpt = _alloc_ptable(&pg);
        if (pdpt == NULL) return NULL;
        pml4->table[pml4i] = PTE_ADDR(pg->phys_addr) | flags;
    }
    vm_ptable_t *pdpt = (vm_ptable_t*)vm_ptov(PTE_ADDR(pml4->table[pml4i]));

    if (!(pdpt->table[pdpti] & PDE_PRESENT)) {
        if (!create) return NULL;
        vm_page_t *pg;
        vm_ptable_t *pd = _alloc_ptable(&pg);
        if (pd == NULL) return NULL;
        pdpt->table[pdpti] = PTE_ADDR(pg->phys_addr) | flags;
    }
    vm_ptable_t *pd = (vm_ptable_t*)vm_ptov(PTE_ADDR(pdpt->table[pdpti]));

    if (!(pd->table[pdi] & PDE_PRESENT)) {
        if (!create) return NULL;
        vm_page_t *pg;
        vm_ptable_t *pt = _alloc_ptable(&pg);
        if (pt == NULL) return NULL;
        pd->table[pdi] = PTE_ADDR(pg->phys_addr) | flags;
    }
    return (vm_ptable_t*)vm_ptov(PTE_ADDR(pd->table[pdi]));
}

static vm_ptable_t *
_pmap_find_pt(const vm_pmap_t *vpm, vm_addr_t addr)
{
    return _pmap_get_pt((vm_pmap_t *)vpm, addr, FALSE);
}

static bool
_ptable_is_empty(vm_ptable_t *pt)
{
    for (int i = 0; i < PTABLE_ENTRIES; i++) {
        if (pt->table[i] & PTE_PRESENT)
            return FALSE;
    }
    return TRUE;
}

vm_page_t *
pmap_get_page(const vm_pmap_t *pmap, vm_addr_t addr)
{
    int n = PAGE_NUM(vm_pmap_phys(pmap, addr));
    return &vm_pages[n];
}

/*========================================================================
 * Obsługa stron.
 */


/**
 * Tłumaczy adres fizyczny na wirtualny.
 * @bug Obecnie działa jedynie dla pamięci wirtualnej jądra.
 * @warning Zdefiniowane jedynie dla prawidłowych adresów.
 */
vm_addr_t
vm_ptov(vm_paddr_t pa)
{
    int n = PAGE_NUM(pa);
    return vm_pages[n].kvirt_addr + PAGE_OFF(pa);
}



/*========================================================================
 * Pomocnicze funkcje.
 */

/**
 * Przydziela stronę na tablicę lub katalog.
 * @return strona, lub NULL gdy nie udało się przydzielić.
 */
vm_ptable_t *
_alloc_ptable(vm_page_t **pgp)
{
    vm_ptable_t *res;
    vm_seg_alloc(vm_kspace.seg_data, PAGE_SIZE, &res);
    vm_page_t *pg = pmap_get_page(&vm_kspace.pmap, (vm_addr_t)res);
    pg->kvirt_addr = (vm_addr_t) res;
    if (pgp) *pgp = pg;
    return res;
}


vm_page_t *
_alloc_page()
{
    vm_page_t *p = vm_alloc_page();
    p->kvirt_addr = kseg_data.end;
    kseg_data.end += PAGE_SIZE;
    kseg_data.size += PAGE_SIZE;
    vm_pmap_insert(&vm_kspace.pmap, p, p->kvirt_addr,
         VM_PROT_RWX | VM_PROT_SYSTEM);
    memzero((void*)p->kvirt_addr, PAGE_SIZE);
    return p;
}


vm_page_t *
_alloc_kpage()
{
    int n = PAGE_NUM(kseg_data.end - kstart + kbootstrap);
    vm_page_t *p = &vm_pages[n];
    p->kvirt_addr = kseg_data.end;
    kseg_data.end += PAGE_SIZE;
    kseg_data.size += PAGE_SIZE;
    list_remove(&vm_free_pages, p);
    memzero((void*)p->kvirt_addr, PAGE_SIZE);
    return p;
}

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
#include <sys/vm/vm_pmap.h>
#include <sys/vm/vm_lpool.h>
#include <sys/vm/vm_internal.h>
#include <sys/utils.h>
#include <sys/string.h>
#include <machine/memory.h>
#include <machine/cpu.h>
#include <machine/io.h>

size_t vm_physmem_max;
size_t vm_physmem_free;


static void _pmap_insert_pte(vm_pmap_t *t, vm_page_t *p, vm_addr_t va);
static void _pmap_insert_pte_(vm_pmap_t *vpm, vm_ptable_t *pt, vm_addr_t va);
static vm_page_t * pmap_get_page(const vm_pmap_t *pmap, vm_addr_t addr);

static vm_ptable_t *_alloc_ptable(vm_page_t **pg);
static bool find_page(const vm_page_t *pg, uintptr_t paddr);

/// Wska¼nik do tablicy opisu stron.
static vm_page_t *vm_pages;
/// Ilosc stron przeznaczona na opis.
static int vm_pages_size;

#define _GLOBAL(a,f) (VM_SPACE_DATA_BEGIN <= a && a < VM_SPACE_DATA_END)? f : 0

/*========================================================================
 * Stronicowanie,
 */


///
void
vm_pmap_switch(const vm_pmap_t *pm)
{
    cpu_set_cr3(pm->physdir);
}

/// W³±cza stronicowanie.
void
vm_enable_paging()
{
    cpu_set_cr0(cpu_get_cr0() | CR0_PG);
}

/// Wy³±cza stronicowanie.
void
vm_disable_paging()
{
    cpu_set_cr0(cpu_get_cr0() & ~CR0_PG);
}


/*========================================================================
 * Inicjalizacja.
 */

static size_t _avail_physmem(void);
static void create_kernel_space(void);
static void create_kernel_data(void);
static void initialize_internal(void);
static void _collect_free_pages(void);

void
vm_low_init()
{
    vm_physmem_max = _avail_physmem();
    _collect_free_pages();
    create_kernel_space();
    create_kernel_data();
    initialize_internal();

}

vm_page_t *page;

/// Inicjalizuje przestrzeñ adresow± j±dra.
void
create_kernel_space()
{
    // ustaw segmenty.
    vm_segment_create(&vm_kspace.seg_code, &vm_kspace, VM_SPACE_CODE_BEGIN,
        VM_SPACE_CODE_SIZE, VM_SPACE_CODE_SIZE);
    vm_segment_create(&vm_kspace.seg_data, &vm_kspace, VM_SPACE_DATA_BEGIN,
        0, VM_SPACE_DATA_SIZE);
    vm_segment_create(&vm_kspace.seg_stack, &vm_kspace, VM_SPACE_STACK_BEGIN,
        0, VM_SPACE_STACK_SIZE);

    // stwórz odwzorowywanie j±dra.
    vm_pmap_t *kmap = &vm_kspace.pmap;
    mem_zero(kmap, sizeof(vm_pmap_t));
    list_create(&kmap->pages, offsetof(vm_page_t, L_pages), FALSE);
    kmap->keep_ptes = TRUE;
    page = vm_alloc_page();
    page->kvirt_addr = page->phys_addr;
    kmap->physdir = page->phys_addr;
    kmap->pdir = (vm_ptable_t *) page->phys_addr;
    uintptr_t *table_dir = (uintptr_t*) page->phys_addr;

    page = vm_alloc_page();
    page->kvirt_addr = page->phys_addr;

    // Uzupe³niamy katalog stron.
    table_dir[0] = page->phys_addr | PDE_PRESENT | PDE_RW | PDE_G;
    for (int i = 1; i < 1024; i++)
        table_dir[i] = 0;

    // Odwzorowywujemy jeden-do-jednego pierwsze 4MB
    uintptr_t *table = (uintptr_t*) page->phys_addr;
    for (int i = 0; i < 1024; i++) {
        table[i] = PAGE_ADDR(i) | PTE_PRESENT | PTE_RW | PTE_G;
    }

    // W³±czamy stronicowanie.
    vm_pmap_switch(kmap);
    vm_enable_paging();
}

void
create_kernel_data()
{

    // Tworzymy tablice stron dla pamiêci j±dra, i rêcznie rozszerzamy
    // stertê j±dra.
    vm_pmap_t *kmap = &vm_kspace.pmap;
    vm_segment_t *kdata = &vm_kspace.seg_data;
    vm_addr_t vaddr = kdata->base;

    // przydzielamy pamiêæ na pierwsz± tablicê stron.
    vm_page_t *page = vm_alloc_page();
    page->kvirt_addr = kdata->base + kdata->size;
    uintptr_t *table = (uintptr_t*) page->phys_addr;
    table[0] = page->phys_addr | PDE_PRESENT | PDE_RW | PDE_US;
    _pmap_insert_pte(kmap, page, vaddr);
    kdata->size += PAGE_SIZE;
    vaddr += PAGE_SIZE*1024;
    for (int i = 1; i < VM_SPACE_KERNEL_PTABLES; i++ ){
        page = vm_alloc_page();
        mem_zero((void*)page->phys_addr, PAGE_SIZE);
        page->kvirt_addr = kdata->base + kdata->size;
        table[i] = page->phys_addr | PDE_PRESENT | PDE_RW | PDE_US;
        kdata->size += PAGE_SIZE;
        _pmap_insert_pte(kmap, page, vaddr);
        vm_pmap_insert(kmap, page, page->kvirt_addr);
        vaddr += PAGE_SIZE*1024;
    }
}

void
initialize_internal()
{
    vm_pmap_t *kmap = &vm_kspace.pmap;
    vm_segment_t *kdata = &vm_kspace.seg_data;

    // przydzielamy miejsce na poczatkowe regiony
    vm_page_t *page = vm_alloc_page();
    page->kvirt_addr = kdata->base + kdata->size;
    kdata->size += PAGE_SIZE;
    vm_pmap_insert(kmap, page, page->kvirt_addr);
    vm_lpool_create_(&vm_unused_regions, offsetof(vm_region_t,L_regions),
            sizeof(vm_region_t), VM_LPOOL_NORMAL, (void*)page->kvirt_addr);
    page = vm_alloc_page();
    page->kvirt_addr = kdata->base + kdata->size;
    kdata->size += PAGE_SIZE;
    vm_pmap_insert(kmap, page, page->kvirt_addr);
    vm_lpool_insert_empty(&vm_unused_regions, (void*)page->kvirt_addr);

    vm_region_t *reg = vm_lpool_alloc(&vm_unused_regions);

    reg->begin = kdata->base;
    reg->size = kdata->size;
    reg->end = reg->begin + reg->size;
    reg->segment = kdata;
    list_insert_head(&kdata->regions, reg);
}

/// Wykrywa ilo¶æ zainstalowanej pamiêci RAM.
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

/// Zbiera wolne strony.
void
_collect_free_pages()
{
    extern int kernel_end;
    vm_addr_t paddr;
    vm_addr_t code_end = (PAGE_NUM(&kernel_end) + 1);
    // ilo¶æ stron przeznaczonych na administracjê.
    vm_pages_size = ((sizeof(vm_page_t) * vm_physmem_max + 4095) >> PAGE_SHIFT);

    // obliczamy ilo¶æ wolnych stron.
    vm_physmem_free =  vm_physmem_max - code_end;
    vm_physmem_free -= vm_pages_size;

    // adres tablicy.
    code_end <<= PAGE_SHIFT;
    vm_pages = (vm_page_t*) code_end;
    code_end += PAGE_SIZE*vm_pages_size;

    paddr = 0;
    list_create( &vm_free_pages, offsetof(vm_page_t, L_pages), FALSE);
    for (int i = 0; i < vm_physmem_free; i++, paddr += PAGE_SIZE) {
        vm_pages[i].phys_addr = paddr;
        if (paddr > code_end) {
            vm_pages[i].kvirt_addr = 0;
            vm_pages[i].flags = PAGE_FREE;
            vm_pages[i].refcnt = 0;
            list_insert_tail(&vm_free_pages, &vm_pages[i]);
        } else {
            vm_pages[i].kvirt_addr = paddr;
            vm_pages[i].flags = 0;
        }
    }
}



/*========================================================================
 * Obs³uga vm_pmap_t
 */

static vm_ptable_t *_pmap_pde(const vm_pmap_t *vpm, vm_paddr_t addr);
static vm_ptable_t *_pmap_dir(const vm_pmap_t *vpm, vm_paddr_t addr);

/**
 * Inicjalizuje odwzorowanie stron.
 * @param vpm odwzorowanie stron.
 * @return FALSE wtedy i tylko wtedy, gdy nie mo¿na by³o przydzieliæ
 *         strony na nowy katalog stron.
 */

bool
vm_pmap_init(vm_pmap_t *vpm)
{
    vm_page_t *page;
    mem_zero(vpm, sizeof(*vpm));
    vpm->pdir = (vm_ptable_t*) _alloc_ptable(&page);
    vpm->physdir = page->phys_addr;
    vpm->keep_ptes = FALSE;
    list_create(&vpm->pages, offsetof(vm_page_t,L_pages), FALSE);
    return (vpm->pdir != 0);
}


/**
 * Ustawia odwzorowywanie strony.
 * @param vpm odwzorowanie stron.
 * @param p strona.
 * @param va wirtualny adres strony.
 * @return FALSE wtedy i tylko wtedy gdy mo¿na by³o przydzieliæ pamiêci
 *         na now± tablicê stron.
 */
bool
vm_pmap_insert(vm_pmap_t *vpm, vm_page_t *p, vm_addr_t va)
{
    bool _G = _GLOBAL(va, PTE_G);
    vm_addr_t pa = p->phys_addr;
    int pte = PAGE_TBL(va);
    vm_ptable_t *pt = _pmap_pde(vpm, va);
    if (pt == NULL) {
        pt = _alloc_ptable(NULL);
        if (pt == NULL) return FALSE;
        _pmap_insert_pte_(vpm, pt, va);
    }
    p->refcnt++;
    if (!_G) vpm->pdircount[PAGE_DIR(va)]++;
    pt->table[pte] = PTE_ADDR(pa) | PTE_PRESENT | PTE_RW | _G;
    list_insert_tail(&vpm->pages, p);
    return TRUE;
}

/**
 * Ustawia odwzorowywanie strony.
 * @param vpm odwzorowanie stron.
 * @param vp adres fizyczny strony.
 * @param va wirtualny adres strony.
 * @return FALSE wtedy i tylko wtedy gdy mo¿na by³o przydzieliæ pamiêci
 *         na now± tablicê stron.
 *
 * Procedura jest wykorzystywana przez swój odpowiednik dla vm_page_t.
 * Istnieje poniewa¿ nie dla wszystkich stron robimy opis. Np
 * Nie bêdziemy zarz±dzañ stronami z kodem j±dra.
 */
bool
vm_pmap_insert_(vm_pmap_t *vpm, vm_paddr_t pa, vm_addr_t va)
{
    return vm_pmap_insert(vpm, &vm_pages[PAGE_NUM(pa)], va);
}

void
vm_pmap_fill(vm_pmap_t *pmap, vm_addr_t addr, vm_size_t size)
{
    for (size+=addr; addr < size; addr += PAGE_SIZE) {
        vm_page_t *p = vm_alloc_page();
        vm_pmap_insert(pmap, p, addr);
    }
}


void
vm_pmap_map(vm_pmap_t *dst_pmap, vm_addr_t dst_addr, const vm_pmap_t *src_pmap,
    vm_addr_t src_addr, vm_size_t size)
{
    for (size+=dst_addr; dst_addr < size; dst_addr += PAGE_SIZE) {
        vm_page_t *page = pmap_get_page(src_pmap, src_addr);
        vm_pmap_insert(dst_pmap, page, dst_addr);
        src_addr += PAGE_SIZE;
    }
}

void
vm_pmap_clone(vm_pmap_t *dst, const vm_pmap_t *src)
{
        vm_pmap_init(dst);
        vm_ptable_t *sdir = src->pdir;
        vm_ptable_t *ddir = dst->pdir;
        mem_cpy(dst->pdir, src->pdir, PAGE_SIZE);
        for (int i = 0; i < PAGE_TBL(VM_SPACE_DATA_BEGIN); i++) {
            if (sdir->table[i] & PDE_PRESENT) {
                vm_page_t *page;
                vm_ptable_t *newpt = _alloc_ptable(&page);
                vm_ptable_t *oldpt = _pmap_pde(src, sdir->table[i]);
                mem_cpy(newpt, oldpt, PAGE_SIZE);
                ddir->table[i] = PTE_FLAGS(sdir->table[i]) | page->phys_addr;
            }
        }
}

/**
 * Zwalnia stronê.
 */
bool
vm_pmap_remove(vm_pmap_t *pmap, vm_addr_t va)
{
    int pte = PAGE_TBL(va);
    int pde = PAGE_DIR(va);
    int _G = _GLOBAL(va, 1);
    vm_ptable_t *pt = _pmap_pde(pmap, va);
    if (pt == NULL) {
        return FALSE;
    }
    pt->table[pte] = PTE_ADDR(pt->table[pte]);
    vm_page_t *pg = list_find(&pmap->pages, find_page, pt->table[pte]);
    KASSERT(pg != NULL);
    list_remove(&pmap->pages, pg);
    pt->table[pte] = 0;
    if (!_G) {
        if (--pmap->pdircount[pde] == 0) {
            vm_segment_free(&vm_kspace.seg_data, (vm_addr_t)pt, PAGE_SIZE);
            pmap->pdir->table[pde] = 0;
        }
    }
    return TRUE;
}


void
vm_pmap_erase(vm_pmap_t *pmap, vm_addr_t addr, vm_size_t size)
{
    for (size += addr; addr < size; addr += PAGE_SIZE) {
        vm_pmap_remove(pmap, addr);
    }
}



/**
 * T³umaczy adres wirtualny na fizyczny.
 * @param vpm odwzorowanie stron.
 * @param va adres wirtualny.
 * @return adres fizyczny
 * @warning procedura jest zdefiniowiowana jedynie dla istniej±cych
 *          w danym odwzorowaniu adresów wirtualnych.
 */

vm_paddr_t
vm_pmap_phys(const vm_pmap_t *vpm, vm_addr_t va)
{
    int pte = PAGE_TBL(va);
    uintptr_t off = PAGE_OFF(va);
    vm_ptable_t *pt = _pmap_pde(vpm, va);
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
    vm_ptable_t *pt = _pmap_pde(vpm, va);
    if (!pt) return FALSE;
    return pt->table[pte] & PTE_PRESENT;
}



/**
 * Wstawia tablicê stron w katalog.
 * @param vpm odwzorowanie stron.
 * @param p przydzielona strona do tablicy.
 * @param va pocz±tek 4MB przedzia³u t³umaczonego przez dan± tablicê.
 */
void
_pmap_insert_pte(vm_pmap_t *vpm, vm_page_t *p, vm_addr_t va)
{
    _pmap_insert_pte_(vpm, (vm_ptable_t*)p->phys_addr, va);
}

/**
 * Wstawia tablicê stron w katalog.
 * @param vpm odwzorowanie stron.
 * @param pt tablica stron.
 * @param va pocz±tek 4MB przedzia³u t³umaczonego przez dan± tablicê.
 */
void
_pmap_insert_pte_(vm_pmap_t *vpm, vm_ptable_t *pt, vm_addr_t va)
{
    bool _G = _GLOBAL(va, PDE_G);
    int pde = PAGE_DIR(va);
//     vm_ptable_t *pdir = _pmap_dir(vpm, va);
    vpm->pdir->table[pde] = PTE_ADDR(pt) | PDE_PRESENT | PDE_RW | _G;
}


vm_ptable_t *
_pmap_pde(const vm_pmap_t *vpm, vm_paddr_t addr)
{
    int pde = PAGE_DIR(addr);
    vm_ptable_t *dir = _pmap_dir(vpm, addr);
    if (dir->table[pde] & PDE_PRESENT) {
        vm_ptable_t *pt = (vm_ptable_t*) vm_ptov(PTE_ADDR(dir->table[pde]));
        return pt;
    } else {
        return NULL;
    }
}

vm_ptable_t *
_pmap_dir(const vm_pmap_t *vpm, vm_paddr_t addr)
{
//     vm_ptable_t *p = (vm_ptable_t*) vm_ptov(PTE_ADDR(vpm->pdir));
    return vpm->pdir;
}

vm_page_t *
pmap_get_page(const vm_pmap_t *pmap, vm_addr_t addr)
{
    int n = PAGE_NUM(vm_pmap_phys(pmap, addr));
    return &vm_pages[n];
}

/*========================================================================
 * Obs³uga stron.
 */


/**
 * T³umaczy adres fizyczny na wirtualny.
 * @bug Obecnie dzia³a jedynie dla pamiêci wirtualnej j±dra.
 * @warning Zdefiniowane jedynie dla prawid³owych adresów.
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
 * Przydziela stronê na tablicê lub katalog.
 * @return strona, lub NULL gdy nie uda³o siê przydzieliæ.
 */
vm_ptable_t *
_alloc_ptable(vm_page_t **pgp)
{
#if 0
    vm_page_t *p = vm_kernel_alloc_page();
    if (p) {
        mem_zero((void*)p->phys_addr, PAGE_SIZE);
        return (vm_ptable_t*)p->phys_addr;
    } else {
        return NULL;
    }
#endif
    vm_ptable_t *res;
    vm_segment_alloc(&vm_kspace.seg_data, PAGE_SIZE, &res);
    vm_page_t *pg = pmap_get_page(&vm_kspace.pmap, (vm_addr_t)res);
    pg->kvirt_addr = (vm_addr_t) res;
    if (pgp) *pgp = pg;
    return res;
}


bool
find_page(const vm_page_t *pg, uintptr_t paddr)
{
    return (pg->phys_addr == paddr);

}

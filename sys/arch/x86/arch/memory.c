#include <sys/types.h>
#include <sys/kprintf.h>
#include <sys/vm.h>
#include <sys/vm/vm_pmap.h>
#include <sys/utils.h>
#include <sys/string.h>
#include <machine/memory.h>
#include <machine/cpu.h>
#include <machine/io.h>

size_t vm_physmem_max;
size_t vm_physmem_free;

vm_space_t vm_kspace;

static void _pmap_insert_pte(vm_pmap_t *t, vm_page_t *p, vm_addr_t va);
static void _pmap_insert_pte_(vm_pmap_t *vpm, vm_ptable_t *pt, vm_addr_t va);

static vm_ptable_t *_alloc_ptable(void);

/// Wska¼nik do tablicy opisu stron.
static vm_page_t *vm_pages;
/// Ilosc stron przeznaczona na opis.
static int vm_pages_size;

/*========================================================================
 * Stronicowanie,
 */


/// 
void
vm_pmap_switch(const vm_pmap_t *pm)
{
    cpu_set_cr3(pm->pdir);
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
static void _set_system_vspace(void);
static void _collect_free_pages(void);

#if 0
static void _check(uintptr_t p);

void
_check(uintptr_t p)
{
    bool b = vm_pmap_is_avail(&vm_kspace.pmap, p);
    if (b) {
        vm_addr_t v = vm_pmap_phys(&vm_kspace.pmap, p);
        vm_paddr_t pp = vm_ptov(v);
        kprintf("%p -> %p -> %p\n", p, v, pp);
    } else {
        kprintf("%p is invalid\n", p);
    }
}

#endif

void
vm_low_init()
{
    vm_physmem_max = _avail_physmem();
    _collect_free_pages();
    _set_system_vspace();
#if 0
    vm_page_t *p = vm_kernel_alloc_page();
    _check(p->kvirt_addr);
#endif
}

vm_page_t *page;

/// Inicjalizuje przestrzeñ adresow± j±dra.
void
_set_system_vspace()
{
    // ustaw segmenty.
    vm_segment_create(&vm_kspace.seg_code, &vm_kspace, VM_SPACE_CODE_BEGIN,
        VM_SPACE_CODE_SIZE, VM_SPACE_CODE_SIZE);
    vm_segment_create(&vm_kspace.seg_data, &vm_kspace, VM_SPACE_DATA_BEGIN,
        0, VM_SPACE_DATA_SIZE);
    vm_segment_create(&vm_kspace.seg_stack, &vm_kspace, 0,
        0, 0);

    // stwórz odwzorowywanie j±dra.
    vm_pmap_t *kmap = &vm_kspace.pmap;
    list_create(&kmap->pages, offsetof(vm_page_t, L_pages), FALSE);
    page = vm_alloc_page();
    page->kvirt_addr = page->phys_addr;
    kmap->pdir = page->phys_addr;
    uintptr_t *table_dir = (uintptr_t*) page->phys_addr;

    page = vm_alloc_page();
    page->kvirt_addr = page->phys_addr;

    // Uzupe³niamy katalog stron.
    table_dir[0] = page->phys_addr | PDE_PRESENT | PDE_RW | PDE_US;
    for (int i = 1; i < 1024; i++) 
        table_dir[i] = 0;

    // Odwzorowywujemy jeden-do-jednego pierwsze 4MB
    uintptr_t *table = (uintptr_t*) page->phys_addr;
    for (int i = 0; i < 1024; i++) {
        table[i] = PAGE_ADDR(i) | PTE_PRESENT | PTE_RW | PTE_US;
    }

    // W³±czamy stronicowanie.
    vm_pmap_switch(kmap);
    vm_enable_paging();

    // Tworzymy tablice stron dla pamiêci j±dra, i rêcznie rozszerzamy
    // stertê j±dra.
    vm_segment_t *kdata = &vm_kspace.seg_data;
    vm_addr_t vaddr = kdata->base;

    // przydzielamy pamiêæ na pierwsz± tablicê stron.
    page = vm_alloc_page();
    page->kvirt_addr = kdata->base + kdata->size;
    table = (uintptr_t*) page->phys_addr;
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
    // przydzielamy miejsce na poczatkowe regiony
    list_create(&vm_unused_regions, offsetof(vm_region_t, L_regions), FALSE);
    page = vm_alloc_page();
    page->kvirt_addr = kdata->base + kdata->size;
    kdata->size += PAGE_SIZE;

    vm_region_t *regs = (vm_region_t*)page->phys_addr;
    regs->begin = kdata->base;
    regs->size = kdata->size;
    list_insert_tail(&kdata->regions, regs);
    regs++;
    for (int i = sizeof(vm_region_t); i < PAGE_SIZE; i+= sizeof(vm_region_t)) {
        list_insert_tail(&vm_unused_regions, regs);
        regs++;
    }
    kprintf("VM: available memory: %ukB (%u pages)\n",
        vm_physmem_free*4, vm_physmem_free);

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
    vm_addr_t code_end = (BASE_ADDR(&kernel_end) + 1);
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
            list_insert_tail(&vm_free_pages, &vm_pages[i]);
        } else {
            vm_pages[i].kvirt_addr = paddr;
            vm_pages[i].flags = 0;
        }
    }
    kprintf("VM: page administration region: %p+%u kB (%u pages)\n", vm_pages,
            vm_pages_size*4, vm_pages_size);
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
    mem_zero(vpm, sizeof(*vpm));
    vpm->pdir = vm_vtop((uintptr_t)_alloc_ptable());
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
    list_insert_tail(&vpm->pages, p);
//     kprintf("adding %p as %p\n", va, p->phys_addr);
    return vm_pmap_insert_(vpm, p->phys_addr, va);
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
//     int pde = PAGE_DIR(va);
    int pte = PAGE_TBL(va);
    vm_ptable_t *pt = _pmap_pde(vpm, va); //(vm_ptable_t*) PTE_ADDR(vpm->pdir->table[pde]);
//     kprintf("[%p as %p] pde=%p pte=%u\n", va, pa, pt, pte);
    if (pt == NULL) {
//         kprintf("[need new pte]\n");
        // Je¿eli dana tablica stron nie istnieje, to przydziel j±.
        pt = _alloc_ptable();
        if (pt == NULL) return FALSE;
        _pmap_insert_pte_(vpm, pt, va);
    }
    pt->table[pte] = PTE_ADDR(pa) | PTE_PRESENT | PTE_RW | PTE_US;
//     kprintf("pt->table[%u] = %p\n", pt->table[pte]);
    return TRUE;

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
    int pde = PAGE_DIR(va);
    vm_ptable_t *pdir = _pmap_dir(vpm, va);
    pdir->table[pde] = PTE_ADDR(pt) | PDE_PRESENT | PDE_RW | PDE_US;
//     kprintf("pdir->table[%u] = %p\n", pde, pdir->table[pde]);
//     kprintf("inserted pdir=%p pde=%u pt=%p <- %p\n",pdir, pde, pt,
//         PTE_ADDR(pdir->table[pde]));
}

/**
 * T³umaczy adres fizyczny na wirtualny.
 * @bug Obecnie dzia³a jedynie dla pamiêci wirtualnej j±dra.
 * @warning Zdefiniowane jedynie dla prawid³owych adresów.
 */
vm_addr_t
vm_ptov(vm_paddr_t pa)
{
    int n = BASE_ADDR(pa);
//     kprintf("QUERY n=%u(0x%x) %p - %p \n", n, n, pa, &vm_pages[n]);
    return vm_pages[n].kvirt_addr + PAGE_OFF(pa);
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
    vm_ptable_t *p = (vm_ptable_t*) vm_ptov(PTE_ADDR(vpm->pdir));
    return p;
}



/*========================================================================
 * Pomocnicze funkcje.
 */

/**
 * Przydziela stronê na tablicê lub katalog.
 * @return strona, lub NULL gdy nie uda³o siê przydzieliæ.
 */
vm_ptable_t *
_alloc_ptable()
{
    vm_page_t *p = vm_kernel_alloc_page();
    if (p) {
        mem_zero((void*)p->phys_addr, PAGE_SIZE);
        return (vm_ptable_t*)p->phys_addr;
    } else {
        return NULL;
    }
}


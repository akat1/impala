#include <sys/types.h>
#include <sys/kprintf.h>
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

vm_space_t vm_kspace;

static void _pmap_insert_pte(vm_pmap_t *t, vm_page_t *p, vm_addr_t va);
static void _pmap_insert_pte_(vm_pmap_t *vpm, vm_ptable_t *pt, vm_addr_t va);

static vm_ptable_t *_alloc_ptable(void);

/// Wska�nik do tablicy opisu stron.
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

/// W��cza stronicowanie.
void
vm_enable_paging()
{
    cpu_set_cr0(cpu_get_cr0() | CR0_PG);
}

/// Wy��cza stronicowanie.
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

/// Inicjalizuje przestrze� adresow� j�dra.
void
create_kernel_space()
{
    // ustaw segmenty.
    vm_segment_create(&vm_kspace.seg_code, &vm_kspace, VM_SPACE_CODE_BEGIN,
        VM_SPACE_CODE_SIZE, VM_SPACE_CODE_SIZE);
    vm_segment_create(&vm_kspace.seg_data, &vm_kspace, VM_SPACE_DATA_BEGIN,
        0, VM_SPACE_DATA_SIZE);
    vm_segment_create(&vm_kspace.seg_stack, &vm_kspace, 0,
        0, 0);

    // stw�rz odwzorowywanie j�dra.
    vm_pmap_t *kmap = &vm_kspace.pmap;
    list_create(&kmap->pages, offsetof(vm_page_t, L_pages), FALSE);
    page = vm_alloc_page();
    page->kvirt_addr = page->phys_addr;
    kmap->pdir = page->phys_addr;
    uintptr_t *table_dir = (uintptr_t*) page->phys_addr;

    page = vm_alloc_page();
    page->kvirt_addr = page->phys_addr;

    // Uzupe�niamy katalog stron.
    table_dir[0] = page->phys_addr | PDE_PRESENT | PDE_RW | PDE_US;
    for (int i = 1; i < 1024; i++) 
        table_dir[i] = 0;

    // Odwzorowywujemy jeden-do-jednego pierwsze 4MB
    uintptr_t *table = (uintptr_t*) page->phys_addr;
    for (int i = 0; i < 1024; i++) {
        table[i] = PAGE_ADDR(i) | PTE_PRESENT | PTE_RW | PTE_US;
    }

    // W��czamy stronicowanie.
    vm_pmap_switch(kmap);
    vm_enable_paging();
}

void
create_kernel_data()
{
   
    // Tworzymy tablice stron dla pami�ci j�dra, i r�cznie rozszerzamy
    // stert� j�dra.
    vm_pmap_t *kmap = &vm_kspace.pmap;
    vm_segment_t *kdata = &vm_kspace.seg_data;
    vm_addr_t vaddr = kdata->base;

    // przydzielamy pami�� na pierwsz� tablic� stron.
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
    list_insert_head(&kdata->regions, reg);
}

/// Wykrywa ilo�� zainstalowanej pami�ci RAM.
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
    // ilo�� stron przeznaczonych na administracj�.
    vm_pages_size = ((sizeof(vm_page_t) * vm_physmem_max + 4095) >> PAGE_SHIFT);

    // obliczamy ilo�� wolnych stron.
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
}



/*========================================================================
 * Obs�uga vm_pmap_t
 */

static vm_ptable_t *_pmap_pde(const vm_pmap_t *vpm, vm_paddr_t addr);
static vm_ptable_t *_pmap_dir(const vm_pmap_t *vpm, vm_paddr_t addr);

/**
 * Inicjalizuje odwzorowanie stron.
 * @param vpm odwzorowanie stron.
 * @return FALSE wtedy i tylko wtedy, gdy nie mo�na by�o przydzieli�
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
 * @return FALSE wtedy i tylko wtedy gdy mo�na by�o przydzieli� pami�ci
 *         na now� tablic� stron.
 */
bool
vm_pmap_insert(vm_pmap_t *vpm, vm_page_t *p, vm_addr_t va)
{
    list_insert_tail(&vpm->pages, p);
    return vm_pmap_insert_(vpm, p->phys_addr, va);
}


/**
 * Ustawia odwzorowywanie strony.
 * @param vpm odwzorowanie stron.
 * @param vp adres fizyczny strony.
 * @param va wirtualny adres strony.
 * @return FALSE wtedy i tylko wtedy gdy mo�na by�o przydzieli� pami�ci
 *         na now� tablic� stron.
 *
 * Procedura jest wykorzystywana przez sw�j odpowiednik dla vm_page_t.
 * Istnieje poniewa� nie dla wszystkich stron robimy opis. Np
 * Nie b�dziemy zarz�dza� stronami z kodem j�dra.
 */
bool
vm_pmap_insert_(vm_pmap_t *vpm, vm_paddr_t pa, vm_addr_t va)
{
    int pte = PAGE_TBL(va);
    vm_ptable_t *pt = _pmap_pde(vpm, va); //(vm_ptable_t*) PTE_ADDR(vpm->pdir->table[pde]);
    if (pt == NULL) {
        pt = _alloc_ptable();
        if (pt == NULL) return FALSE;
        _pmap_insert_pte_(vpm, pt, va);
    }
    pt->table[pte] = PTE_ADDR(pa) | PTE_PRESENT | PTE_RW | PTE_US;
    return TRUE;

}


/**
 * T�umaczy adres wirtualny na fizyczny.
 * @param vpm odwzorowanie stron.
 * @param va adres wirtualny.
 * @return adres fizyczny
 * @warning procedura jest zdefiniowiowana jedynie dla istniej�cych
 *          w danym odwzorowaniu adres�w wirtualnych.
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
 * Wstawia tablic� stron w katalog.
 * @param vpm odwzorowanie stron.
 * @param p przydzielona strona do tablicy.
 * @param va pocz�tek 4MB przedzia�u t�umaczonego przez dan� tablic�.
 */
void
_pmap_insert_pte(vm_pmap_t *vpm, vm_page_t *p, vm_addr_t va)
{
    _pmap_insert_pte_(vpm, (vm_ptable_t*)p->phys_addr, va);
}

/**
 * Wstawia tablic� stron w katalog.
 * @param vpm odwzorowanie stron.
 * @param pt tablica stron.
 * @param va pocz�tek 4MB przedzia�u t�umaczonego przez dan� tablic�.
 */
void
_pmap_insert_pte_(vm_pmap_t *vpm, vm_ptable_t *pt, vm_addr_t va)
{
    int pde = PAGE_DIR(va);
    vm_ptable_t *pdir = _pmap_dir(vpm, va);
    pdir->table[pde] = PTE_ADDR(pt) | PDE_PRESENT | PDE_RW | PDE_US;
}

/**
 * T�umaczy adres fizyczny na wirtualny.
 * @bug Obecnie dzia�a jedynie dla pami�ci wirtualnej j�dra.
 * @warning Zdefiniowane jedynie dla prawid�owych adres�w.
 */
vm_addr_t
vm_ptov(vm_paddr_t pa)
{
    int n = BASE_ADDR(pa);
    return vm_pages[n].kvirt_addr + PAGE_OFF(pa);
}

/**
 * Kasowanie strony z katalogu.
 */
vm_page_t *
vm_pmap_remove(vm_pmap_t *vpm, vm_addr_t addr)
{
    return 0;
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
 * Przydziela stron� na tablic� lub katalog.
 * @return strona, lub NULL gdy nie uda�o si� przydzieli�.
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


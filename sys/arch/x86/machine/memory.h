#ifndef __MACHINE_MEMORY_H
#define __MACHINE_MEMORY_H

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)


enum {
    VM_SPACE_CODE_BEGIN     = 0x00100000,   // 1MB
    VM_SPACE_CODE_SIZE      = 0x00400000,   // 4MB
    VM_SPACE_DATA_BEGIN     = 0xc0000000,   // 3GB
    VM_SPACE_DATA_SIZE      = 0x3fffffff,   // 1GB
    VM_SPACE_UCODE_BEGIN    = 0x04000000,   // 64MB
    VM_SPACE_UCODE_SIZE     = 0x04400000,   // 4MB
    VM_SPACE_UDATA_BEGIN    = 0x40000000,   // 1GB
};

enum {
    VM_SPACE_KERNEL_PTABLES  = 256,  // ilosc tablic stron 256*4MB = 1GB
};


#ifdef __KERNEL
///@TODO: Lepiej nazwa� i dorobi� kilka makr jeszcze.

/// Wyci�ga z adresu przesuni�cie na stronie.
#define PAGE_OFF(p) (((uintptr_t)p) & 0xfff)
/// Wyci�ga z adresu indeks w katalogu stron.
#define PAGE_DIR(p) (((uintptr_t)p) >> 22)
/// Wyci�ga z adresu indeks w tablicy stron.
#define PAGE_TBL(p) ( (((uintptr_t)p) >> 12) & 0x3ff)

#define BASE_ADDR(p) (((uintptr_t)p) >> PAGE_SHIFT)
#define PTE_ADDR(p) ((uintptr_t)p & 0xfffff000 )


#define PAGE_ADDR(p) (((uintptr_t)p) << PAGE_SHIFT)

#define PAGE_ROUND(a) (((a)+PAGE_SIZE-1)/PAGE_SIZE)

/**
 * Opis bit�w dla wpis�w w tablicy stron (Page Table Entry)
 * 
 * Intel 3A 3-29, Figure 3-14
 */
enum {
    PTE_PRESENT   = 1 << 0,
    PTE_RW        = 1 << 1,
    PTE_US        = 1 << 2,
    PTE_PWT       = 1 << 3,
    PTE_PCD       = 1 << 4,
    PTE_A         = 1 << 5,
    PTE_D         = 1 << 6,
    PTE_PAT       = 1 << 7,
    PTE_G         = 1 << 8,
    PTE_SYS0      = 1 << 9,
    PTE_SYS1      = 1 << 10,
    PTE_SYS2      = 1 << 11,
    PTE_ADDR      = 1 << 12
};

/**
 * Opis bit�w dla wpis�w w katalogu stron (Page Directory Entry)
 *
 * Intel 3A 3-29, Figure 3-14
 */
enum {
    PDE_PRESENT   = 1 << 0,
    PDE_RW        = 1 << 1,
    PDE_US        = 1 << 2,
    PDE_PWT       = 1 << 3,
    PDE_PCD       = 1 << 4,
    PDE_A         = 1 << 5,
    PDE_AVL       = 1 << 6,
    PDE_PS        = 1 << 7,
    PDE_G         = 1 << 8,
    PDE_SYS0      = 1 << 9,
    PDE_SYS1      = 1 << 10,
    PDE_SYS2      = 1 << 11,
    PDE_ADDR      = 1 << 12
};

/**
 * Opisy bit�w dla b��du wyj�tku b��du strony (Page fualt).
 *
 * Intel 3A 5-50
 */
enum PFAULT_ERROR {
    PFE_PRESENT   = 1 << 0,
    PFE_WR        = 1 << 1,
    PFE_US        = 1 << 2,
    PFE_RSVD      = 1 << 3,
    PFE_ID        = 1 << 4
};


enum PAGE_FLAGS {
    PAGE_FREE       = 1 << 0,
    PAGE_BUSY       = 2 << 0,
};


/// Deskryptor strony pami�ci
struct vm_page {
    /// fizyczny adres strony
    vm_paddr_t  phys_addr;
    /// adres w wirtualnej przestrzeni j�dra
    vm_addr_t   kvirt_addr;
    /// opcje
    uint32_t    flags;
    /// w�ze� dla listy stron.
    list_node_t L_pages;        // lista stron.
    /// w�ze� dla listy odwzorowa�, w kt�rych jest strona.
    list_node_t L_pmaps;        // lista odwzorowa� w kt�rych jest strona.
};

/// Tablica (katalog) stron.
struct vm_ptable {
    uintptr_t    table[1024];
} table aligned(PAGE_SIZE);


/// odwzorowanie stron.
struct vm_pmap {
    /// fizyczny adres katalogu stron.
    vm_paddr_t       pdir;      // fizyczny adres katalogu stron.
    /// lista stron w odwzorowaniu.
    list_t           pages;     // wmapowane strony.
};

extern size_t vm_physmem_max;
extern size_t vm_physmem_free;

void vm_low_init(void);
void vm_enable_paging(void);
void vm_disable_paging(void);
bool vm_is_paging(void);

#endif



#endif /* __MACHINE_MEMORY_H */
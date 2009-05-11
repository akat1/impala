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
#ifndef __MACHINE_MEMORY_H
#define __MACHINE_MEMORY_H

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)


enum {
    VM_SPACE_CODE_BEGIN     = 0x00100000,   // 1MB
    VM_SPACE_CODE_SIZE      = 0x00400000,   // 4MB
    VM_SPACE_DATA_BEGIN     = 0xc0000000,   // 3GB
    VM_SPACE_DATA_SIZE      = 0x3fff0000,   // 1GB
    VM_SPACE_DATA_END       = VM_SPACE_DATA_BEGIN + VM_SPACE_DATA_SIZE,
    VM_SPACE_STACK_BEGIN    = VM_SPACE_DATA_END,
    VM_SPACE_STACK_SIZE     = 0x00010000,   // 64KB,
    VM_SPACE_STACK_END      = VM_SPACE_STACK_BEGIN + VM_SPACE_STACK_SIZE,
    VM_SPACE_UCODE_BEGIN    = 0x04000000,   // 64MB
    VM_SPACE_UCODE_SIZE     = 0x04400000,   // 4MB
    VM_SPACE_UDATA_BEGIN    = 0x40000000,   // 1GB
    VM_SPACE_UDATA_SIZE     = 0x7fff0000
};

// Skroci³em nazwy, _BEGIN,_SIZE,_END by³o za d³ugie.
enum {
    VM_SPACE_TEXT       = 0x00100000,   // 1MB
    VM_SPACE_TEXT_S     = 0x00400000,   // 4MB
    VM_SPACE_TEXT_E     = VM_SPACE_TEXT+VM_SPACE_TEXT_S,
    VM_SPACE_DATA       = 0xc0000000,   // 3GB
    VM_SPACE_DATA_S     = 0x40000000,   // 1GB-stos
    VM_SPACE_DATA_E     = VM_SPACE_DATA + VM_SPACE_DATA_S,
    VM_SPACE_UTEXT      = 0x04000000,   // 64MB
    VM_SPACE_UTEXT_S    = 0x04400000,   // 4MB
    VM_SPACE_UDATA      = 0x40000000,   // 1GB
    VM_SPACE_UDATA_S    = 0x80000000,
    VM_SPACE_UDATA_E    = VM_SPACE_UDATA + VM_SPACE_UDATA_S,
};

enum {
    VM_SPACE_KERNEL_PTABLES  = 256,  // ilosc tablic stron 256*4MB = 1GB
};


#ifdef __KERNEL
///@TODO: Lepiej nazwaæ i dorobiæ kilka makr jeszcze.

/// Wyci±ga z adresu przesuniêcie na stronie.
#define PAGE_OFF(p) (((uintptr_t)p) & 0xfff)
/// Wyci±ga z adresu indeks w katalogu stron.
#define PAGE_DIR(p) (((uintptr_t)p) >> 22)
/// Wyci±ga z adresu indeks w tablicy stron.
#define PAGE_TBL(p) ( (((uintptr_t)p) >> 12) & 0x3ff)
#define PAGE_NUM(p) (((uintptr_t)p) >> PAGE_SHIFT)
#define PTE_MASK 0xfffff000
#define PTE_ADDR(p) ((uintptr_t)p & PTE_MASK )
#define PTE_FLAGS(p) ((uintptr_t)p & ~PTE_MASK)

#define PAGE_ADDR(p) (((uintptr_t)p) << PAGE_SHIFT)



/**
 * Opis bitów dla wpisów w tablicy stron (Page Table Entry)
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
 * Opis bitów dla wpisów w katalogu stron (Page Directory Entry)
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
 * Opisy bitów dla b³êdu wyj±tku b³êdu strony (Page fualt).
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


/// Deskryptor strony pamiêci
struct vm_page {
    vm_paddr_t  phys_addr;  ///< fizyczny adres strony
    /// adres w wirtualnej przestrzeni j±dra
    /// u¿ywanie jedynie dla meta-danych VM!
    vm_addr_t   kvirt_addr;
    uint32_t    flags;      ///< opcje
    int         refcnt;
    /// wêze³ dla listy stron.
    list_node_t L_pages;        // lista stron.
    /// wêze³ dla listy odwzorowañ, w których jest strona.
    list_node_t L_pmaps;        // lista odwzorowañ w których jest strona.
};

/// Tablica (katalog) stron.
struct vm_ptable {
    uintptr_t    table[1024];
} table aligned(PAGE_SIZE);


/// odwzorowanie stron.
struct vm_pmap {
    vm_paddr_t      physdir;    //< fizyczny adres katalogu stron.
    vm_ptable_t    *pdir;
    uint16_t        pdircount[1024];

    bool            keep_ptes;  // nie zwalnia pamiêci po pustych PTE
    /// lista stron w odwzorowaniu.
    list_t          pages;      // wmapowane strony.
};

extern size_t vm_physmem_max;
extern size_t vm_physmem_free;

void vm_low_init(void);
void vm_enable_paging(void);
void vm_disable_paging(void);
bool vm_is_paging(void);

#endif



#endif /* __MACHINE_MEMORY_H */

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
#include <sys/utils.h>
#include <sys/vm.h>
#include <sys/vm/vm_lpool.h>

/// Wewnętrzna pula (na stronie).
typedef struct _pool _pool_t;
struct _pool {
    list_node_t     L_pools;
    list_t          elems;
};


static void stabilize_lpool(vm_lpool_t *vlp);
static void init_pool(vm_lpool_t *vlp, _pool_t *pl);
static void grow_lpool(vm_lpool_t *vlp);
static bool is_in_this_pool(uintptr_t paddr, uintptr_t target);

/**
 * Tworzy strukturę lpool.
 * @param vlp wskaźnik do struktury.
 * @param offset przesunięcie uchwytu listy w elementach.
 * @param esize wielkość elementu.
 * @param flags opcje.
 * @param fpool sugerowana strona.
 *
 * Procedura tworzy vlp, jeżeli fpool jest różny od NULL to adres jest
 * traktowany jako adres strony, która jest inicjalizowana jako pula.
 */
void
vm_lpool_create_(vm_lpool_t *vlp, int offset, size_t esize, int flags,
    void *fpool)
{

    vlp->elem_size = esize;
    vlp->elems_per_page = (PAGE_SIZE - sizeof(_pool_t)) / esize;
    vlp->offset = offset;
    vlp->flags = flags;
    list_create(&vlp->empty_pools, offsetof(_pool_t, L_pools), FALSE);
    list_create(&vlp->part_pools, offsetof(_pool_t, L_pools), FALSE);
    list_create(&vlp->full_pools, offsetof(_pool_t, L_pools), FALSE);
    if (fpool) {
        vm_lpool_insert_empty(vlp, fpool);
    }
}

/**
 * Tworzy ustabilizowaną strukturę lpool.
 * @param vlp wskaźnik do struktury.
 * @param offset przesunięcie uchwytu listy w elementach.
 * @param esize wielkość elementu.
 * @param flags opcje.
 *
 * Procedura tworzy strukturę za pomocą vm_lpool_create_(), a następnie
 * uruchamia procedurę stabilizującą.
 */
void
vm_lpool_create(vm_lpool_t *vlp, int offset, size_t esize, int flags)
{
    vm_lpool_create_(vlp, offset, esize, flags, NULL);
    stabilize_lpool(vlp);
}

/**
 * Przydziela element z puli.
 * @param vlp pula elementów.
 * @return przydzielony element.
 */
void*
vm_lpool_alloc(vm_lpool_t *vlp)
{
    void *x;
    _pool_t *pl;
    if ( list_length(&vlp->part_pools) > 0 ) {
        pl = list_extract_first(&vlp->part_pools);
    } else {
        pl = list_extract_first(&vlp->empty_pools);
        if (pl == NULL) {
            grow_lpool(vlp);
            pl = list_extract_first(&vlp->empty_pools);
        }
    }
    x = list_extract_first(&pl->elems);
    if (list_length(&pl->elems) == 0) {
        list_insert_head(&vlp->full_pools, pl);
    } else {
        list_insert_head(&vlp->part_pools, pl);
    }
    stabilize_lpool(vlp);
    return x;
}

/**
 * Zwraca przydzielony element do puli.
 * @param vlp pula elementów.
 * @param x przydzielony element.
 */
void
vm_lpool_free(vm_lpool_t *vlp, void *x)
{
    _pool_t *p;
    p = list_find(&vlp->full_pools, is_in_this_pool, (uintptr_t) x );
    if (p == NULL) {
        p = list_find(&vlp->part_pools, is_in_this_pool, (uintptr_t) x);
        list_remove(&vlp->part_pools, p);
    } else {
        list_remove(&vlp->full_pools, p);
    }
    KASSERT(p != NULL);

    list_insert_head(&p->elems, x);
    if ( list_length(&p->elems) == vlp->elems_per_page ) {
        list_insert_head(&vlp->empty_pools, p);
    } else {
        list_insert_head(&vlp->part_pools, p);
    }
    stabilize_lpool(vlp);
}

/**
 * Niszczy pulę.
 * @param vlp pula.
 * @warning pula musi być w całości opróżniona.
 */
void
vm_lpool_destroy(vm_lpool_t *vlp)
{
    KASSERT( list_length(&vlp->full_pools) == 0 );
    KASSERT( list_length(&vlp->part_pools) == 0 );
}

/**
 * Przydziela stronę jako pulę.
 * @param vlp pula.
 * @param page adres strony.
 */
void
vm_lpool_insert_empty(vm_lpool_t *vlp, void *page)
{
    _pool_t *pl = (_pool_t*) page;
    init_pool(vlp, pl);
    list_insert_head(&vlp->empty_pools, pl);
}

/**
 * Stabilizuje pulę.
 * @param vlp pula.
 */
void
stabilize_lpool(vm_lpool_t *vlp)
{
    size_t emptys = list_length(&vlp->empty_pools);

    if (vlp->flags & VM_LPOOL_PREALLOC && emptys == 0) {
        grow_lpool(vlp);
    } else
    if (emptys > 1) {
        while (list_length(&vlp->empty_pools) > 1) {
            _pool_t *pl = list_extract_last(&vlp->empty_pools);
            vm_seg_free(vm_kspace.seg_data, (vm_addr_t)pl, PAGE_SIZE);
        }
    }
}

void
grow_lpool(vm_lpool_t *vlp)
{
    _pool_t *pl;
    if (vm_seg_alloc(vm_kspace.seg_data, PAGE_SIZE, &pl)) {
        panic("grow_lpool: cannot allocate memory");
    }
    vm_lpool_insert_empty(vlp, pl);
}

/**
 * Inicjalizuje stronę do użytku w puli.
 * @param vlp pula
 * @param pl strona
 */
void
init_pool(vm_lpool_t *vlp, _pool_t *pl )
{
    list_create(&pl->elems, vlp->offset, FALSE);
    char *data = (char*)(pl+1);
    for (int i = 0; i < vlp->elems_per_page; i++) {
        list_insert_tail(&pl->elems, data);
        data += vlp->elem_size;
    }
}

/**
 * Predykat używany do szukania strony zawierającej element.
 * @param addr adres strony
 * @param target adres elementu
 * @return TRUE wtedy i tylko wtedy, gdy dany element jest na danej stronie.
 */
bool
is_in_this_pool(uintptr_t addr, uintptr_t target)
{
    return (addr < target) && (target < addr+PAGE_SIZE);
}

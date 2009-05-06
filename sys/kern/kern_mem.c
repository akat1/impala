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

/*
 * Niniejszy plik zawiera implementacj� alokatora p�ytowego (slab allocator)
 * zaprojektowanego przez Jeff Bonwick'a (Sun Microsystems).
 * http://www.usenix.org/publications/library/proceedings/bos94/full_papers/
 * bonwick.ps
 */

#include <sys/types.h>
#include <sys/kmem.h>
#include <sys/vm.h>
#include <sys/vm/vm_lpool.h>
#include <sys/thread.h>
#include <sys/utils.h>

/*
 * U�ywana strategia do przydzia�u bufor�w opiera si� na przydziale ostatnio
 * u�ywanego buforu, w tym celu u�ywamy list z buforami jak kolejek FIFO.
 * Wierz�, �e ten spos�b przydzia�u mo�e w jakim� stopniu okaza� si� efektywny
 * - ostatnio u�ywany "kawa�ek pami�ci" mo�e by� w cache procesora.
 */

typedef struct kmem_slab kmem_slab_t;
typedef struct kmem_bufctl kmem_bufctl_t;


/// Cache
struct kmem_cache {
    size_t          elem_size;
    const char     *name;
    kmem_ctor_t    *ctor;
    kmem_dtor_t    *dtor;
    list_t          empty_slabs;
    list_t          part_slabs;
    list_t          full_slabs;
    mutex_t         mtx;
    list_node_t     L_caches;
};

/// P�yta
struct kmem_slab {
    list_t          free_bufs;
    list_t          used_bufs;
    size_t          items;
    void           *addr;
    list_node_t     L_slabs;
    kmem_cache_t   *cache;
};

/// Opis kontrolny buforu.
struct kmem_bufctl {
    uint32_t        magic;
    kmem_slab_t     *slab;
    void            *addr;
    list_node_t     L_bufs;
};


enum {
    /// Ograniczenie na elementy grupowane na stronie.
    LARGE_SIZE  = PAGE_SIZE/4,
    /// Pomocnik przy rozpoznawaniu uszkodzonia struktur
    KMEM_BUFCTL_MAGIC = 0xdeadbabe,
    LARGE_BUFS_PREALLOC  = 10,
};



static void cache_init(kmem_cache_t *c);
static void prepare_slab_for_cache(kmem_cache_t *c, kmem_slab_t *s);
static void alloc_init(void);

static kmem_slab_t *get_slab_from_cache(kmem_cache_t *c);
static kmem_bufctl_t *reserve_bufctl(kmem_cache_t *c, kmem_slab_t *s);
static kmem_bufctl_t *get_bufctl_from_ptr(void *ptr);
static void check_cache(kmem_cache_t *c);

/// Pula cache.
static vm_lpool_t lpool_caches;
/// Pula p�yt.
static vm_lpool_t lpool_slabs;
/// Pula opis�w bufor�w.
static vm_lpool_t lpool_bufctls;

static mutex_t global_lock;

/*========================================================================
 * malloc
 */

/// Kube�ek.
typedef struct mem_bucket mem_bucket_t;
struct mem_bucket {
    size_t size;
    const char *name;
    kmem_cache_t *cache;
};

/// Obslugiwane kube�ki pami�ci.
static mem_bucket_t buckets[] = {
    {1 <<  1, "kmem_alloc bucket [2]", NULL},
    {1 <<  2, "kmem_alloc bucket [4]", NULL},
    {1 <<  3, "kmem_alloc bucket [8]", NULL},
    {1 <<  4, "kmem_alloc bucket [16]", NULL},
    {1 <<  5, "kmem_alloc bucket [32]", NULL},
    {1 <<  6, "kmem_alloc bucket [64]", NULL},
    {1 <<  7, "kmem_alloc bucket [128]", NULL},
    {1 <<  8, "kmem_alloc bucket [256]", NULL},
    {1 <<  9, "kmem_alloc bucket [512]", NULL},
    {1 << 10, "kmem_alloc bucket [1024]", NULL},
    {1 << 11, "kmem_alloc bucket [2048]", NULL},
    {1 << 12, "kmem_alloc bucket [4096]", NULL},
    {1 << 13, "kmem_alloc bucket [8192]", NULL},
    {0, NULL, NULL}
};

/**
 * Przydziela pami�� j�dra.
 * @param s wielko��.
 * @param flags opcje przydzia�u.
 * @return wska�nik do przydzielonego elementu.
 */
void*
kmem_alloc(size_t s, int flags)
{
    int i;
    for (i = 0; buckets[i].size && buckets[i].size < s; i++);
    if (buckets[i].size == 0)
        panic("Allocating large regions not possible yet");
    return kmem_cache_alloc(buckets[i].cache, flags);
}

/**
 * Zwalnia pami��.
 * @param ptr wska�nik na pami�� przydzielon� przez kmem_alloc
 */
void
kmem_free(void *ptr)
{
    kmem_bufctl_t *bctl = get_bufctl_from_ptr(ptr);
    kmem_cache_free(bctl->slab->cache, ptr);
}

/// Inicjalizuje kube�ki.
void
alloc_init()
{
    for (int i = 0; buckets[i].size != 0; i++) {
        buckets[i].cache = kmem_cache_create(buckets[i].name,  buckets[i].size,
            NULL, NULL);
    }
}


/*========================================================================
 * kmem_cache_t
 */


/**
 * Tworzy kmem_cache.
 * @param name nazwa cache.
 * @param esize wielko�� elementu.
 * @param ctor wska�nik do konstruktora.
 * @param dtor wska�nik do destruktora.
 */
kmem_cache_t*
kmem_cache_create(const char *name, size_t esize, kmem_ctor_t *ctor,
    kmem_dtor_t *dtor)
{
    mutex_lock(&global_lock);
    kmem_cache_t *cache = vm_lpool_alloc(&lpool_caches);
    cache_init(cache);
    cache->elem_size = esize;
    cache->name = name;
    cache->ctor = ctor;
    cache->dtor = dtor;
    mutex_unlock(&global_lock);
    return cache;
}

/**
 * Niszczy kmem_cache.
 * @param cache wska�nik do cache.
 */
void
kmem_cache_destroy(kmem_cache_t *cache)
{
    // Nigdy nie zamykamy zamk�w w odwrotnej kolejno�ci!
    // Gdyby najpierw zamkn�� zamek globalny, to m�g�oby powsta�
    // zakleszczenie !
    mutex_lock(&cache->mtx);
    mutex_lock(&global_lock);
    KASSERT( list_length(&cache->full_slabs) == 0 );
    KASSERT( list_length(&cache->part_slabs) == 0 );
    while ( list_length(&cache->empty_slabs) > 0 ) {
        kmem_slab_t *slab = list_extract_first(&cache->empty_slabs);
        vm_lpool_free(&lpool_slabs, slab);
    }
    mutex_unlock(&cache->mtx);
    mutex_destroy(&cache->mtx);
    vm_lpool_free(&lpool_caches, cache);
    mutex_unlock(&global_lock);
}

/**
 * Przydziela element z danego kmem_cache.
 * @param cache wska�nik do cache.
 * @param flags opcje przydzia�u.
 * @return wska�nik do przydzielonego elementu.
 */
void *
kmem_cache_alloc(kmem_cache_t *cache, int flags)
{
    mutex_lock(&cache->mtx);
    kmem_slab_t *slab = get_slab_from_cache(cache);
    kmem_bufctl_t *bufctl = reserve_bufctl(cache, slab);
    mutex_unlock(&cache->mtx);
    return bufctl->addr;
}

/**
 * Zwraca element do schowka.
 * @param cache schowek.
 * @param m przydzielony element z danego schowka.
 */
void
kmem_cache_free(kmem_cache_t *cache, void *m)
{
    mutex_lock(&cache->mtx);
    kmem_bufctl_t *bctl = get_bufctl_from_ptr(m);
    // sprawdzamy czy dan� p�yt� nie trzeba przepi��.
    if (list_length(&bctl->slab->free_bufs) == 0) {
        list_remove(&cache->full_slabs, bctl);
        list_insert_head(&cache->part_slabs, bctl); 
    } else
    if (list_length(&bctl->slab->used_bufs) == 1) {
        list_remove(&cache->part_slabs, bctl);
        list_insert_head(&cache->empty_slabs, bctl);
    }
    // przepinamy bufor
    list_remove(&bctl->slab->used_bufs, bctl);
    list_insert_head(&bctl->slab->free_bufs, bctl);
    check_cache(cache);
    mutex_unlock(&cache->mtx);
}


/*========================================================================
 * maintain
 */

/// Inicjalizuje modu�.
void
kmem_init()
{

    vm_lpool_create(&lpool_caches, offsetof(kmem_cache_t, L_caches), 
        sizeof(kmem_cache_t), VM_LPOOL_PREALLOC);
    vm_lpool_create(&lpool_slabs, offsetof(kmem_slab_t, L_slabs),
        sizeof(kmem_slab_t), VM_LPOOL_PREALLOC);
    vm_lpool_create(&lpool_bufctls, offsetof(kmem_bufctl_t, L_bufs),
        sizeof(kmem_bufctl_t), VM_LPOOL_PREALLOC);

    mutex_init(&global_lock, MUTEX_NORMAL);
    alloc_init();
}


/*========================================================================
 * pomocnicze
 */

static void cache_init(kmem_cache_t *c);

/**
 * Inicjaluzuje schowek.
 * @param c wska�nik do schowka
 */
void
cache_init(kmem_cache_t *c)
{
    list_create(&c->empty_slabs, offsetof(kmem_slab_t, L_slabs), FALSE);
    list_create(&c->part_slabs, offsetof(kmem_slab_t, L_slabs), FALSE);
    list_create(&c->full_slabs, offsetof(kmem_slab_t, L_slabs), FALSE);
    mutex_init(&c->mtx, MUTEX_CONDVAR);
    c->elem_size = 0;
}


/**
 * Przygotowuje p�yt� dla danego schowka.
 * @param cache wska�nik do schowka.
 * @param slab wska�nik do p�yty.
 */
void
prepare_slab_for_cache(kmem_cache_t *cache, kmem_slab_t *slab)
{
    list_create(&slab->free_bufs, offsetof(kmem_bufctl_t, L_bufs), FALSE);
    list_create(&slab->used_bufs, offsetof(kmem_bufctl_t, L_bufs), FALSE);
    char *data = (char*)vm_segment_alloc(&vm_kspace.seg_data, PAGE_SIZE);
    KASSERT(data != NULL);
    slab->cache = cache;
    if (cache->elem_size < LARGE_SIZE) {
        slab->addr = data;
        slab->items = PAGE_SIZE / ( sizeof(kmem_bufctl_t) + cache->elem_size );
        for (int i = 0; i < slab->items; i++) {
            kmem_bufctl_t *bctl = (kmem_bufctl_t*) data;
            bctl->addr = data + sizeof(kmem_bufctl_t);
            bctl->magic = KMEM_BUFCTL_MAGIC;
            bctl->slab = slab;
            if (cache->ctor) cache->ctor(bctl->addr);
            list_insert_tail(&slab->free_bufs, bctl);
            data += cache->elem_size + sizeof(kmem_bufctl_t);
        }
    } else {
        panic("Large not supported");
    }
}

/**
 * Pobiera u�yteczn� p�yt� z schowka.
 * @param cache wska�nik do schowka
 * @return wska�nik do p�yty.
 *
 * Je�eli nie ma u�ytecznych p�yt w schowku to przydziela now�.
 */
kmem_slab_t *
get_slab_from_cache(kmem_cache_t *cache)
{
    list_t *ls = NULL;  
    if (list_length(&cache->part_slabs) > 0) {
        ls = &cache->part_slabs;
    } else
    if (list_length(&cache->empty_slabs) > 0) {
        ls = &cache->empty_slabs;
    }
    if (ls) {
        return list_extract_first(ls);
    }
    mutex_lock(&global_lock);
    kmem_slab_t *slab = vm_lpool_alloc(&lpool_slabs);
    mutex_unlock(&global_lock);
    prepare_slab_for_cache(cache, slab);
    return slab;
}

/**
 * Rezerwuje element z p�yty.
 * @param cache wska�nik do schowka.
 * @param slan wska�nik do u�ytecznej p�yty.
 * @return opis bufora
 */
kmem_bufctl_t *
reserve_bufctl(kmem_cache_t *cache, kmem_slab_t *slab)
{
    kmem_bufctl_t *bufctl = list_extract_first(&slab->free_bufs);
    list_insert_tail(&slab->used_bufs, bufctl);
    if (list_length(&slab->free_bufs) == 0) {
        list_insert_tail(&cache->full_slabs, slab);
    } else {
        list_insert_tail(&cache->part_slabs, slab);
    }
    return bufctl;
}

/**
 * Sprawdza poprawno�� schowka.
 * @param cache wska�nik do schowka.
 */
void
check_cache(kmem_cache_t *cache)
{
    while (list_length(&cache->empty_slabs) > 1) {
        kmem_slab_t *slab = list_extract_first(&cache->empty_slabs);
        void *x = NULL;
        if (cache->dtor)
        while ( (x = list_next(&slab->free_bufs, x)) ) {
            cache->dtor(x);
        }
        vm_segment_free(&vm_kspace.seg_data, (vm_addr_t)slab->addr, PAGE_SIZE);
        mutex_lock(&global_lock);
        vm_lpool_free(&lpool_slabs, slab);
        mutex_unlock(&global_lock);
    }
}

/**
 * Zwraca struktur� kontroluj�c� dla danego bufora
 * @param ptr wska�nik na pami�� przydzielon� w jakim� schowku.
 */
kmem_bufctl_t *
get_bufctl_from_ptr(void *ptr)
{
    /*
     * W przysz�o�ci ten schemat mo�e si� zmieni� dla alokacji 
     * du�ych porcji danych, st�d oddzielna procedura.
     */
    kmem_bufctl_t *bctl = ((kmem_bufctl_t*) ptr) - 1;
    KASSERT(bctl->magic == KMEM_BUFCTL_MAGIC );
    return bctl;
}

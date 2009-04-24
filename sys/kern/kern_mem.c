/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/kmem.h>
#include <sys/vm.h>
#include <sys/vm/vm_lpool.h>
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/kprintf.h>

typedef struct kmem_slab kmem_slab_t;
typedef struct kmem_bufctl kmem_bufctl_t;

/// Cache
struct kmem_cache {
    size_t          elem_size;
    kmem_ctor_t     *ctor;
    kmem_dtor_t     *dtor;
    list_t          empty_slabs;
    list_t          part_slabs;
    list_t          full_slabs;
    mutex_t         mtx;
    list_node_t     L_caches;
};

/// P³yta
struct kmem_slab {
    list_t          free_bufs;
    list_t          used_bufs;
    size_t          items;
    void           *addr;
    list_node_t     L_slabs;
};

/// Opis buforu.
struct kmem_bufctl {
    kmem_slab_t     *slab;
    void            *addr;
    list_node_t     L_bufs;
};


enum {
    /// Ograniczenie na elementy grupowane na stronie.
    LARGE_SIZE  = PAGE_SIZE/4,
};



static void cache_init(kmem_cache_t *c);
static void cache_cleanup(kmem_cache_t *c);
static void prepare_slab_for_cache(kmem_cache_t *c, kmem_slab_t *s);
static void alloc_init(void);

static kmem_slab_t *get_slab_from_cache(kmem_cache_t *c);
static kmem_bufctl_t *reserve_bufctl(kmem_cache_t *c, kmem_slab_t *s);
static void check_cache(kmem_cache_t *c);

/// Pula cache.
static vm_lpool_t lpool_caches;
/// Pula p³yt.
static vm_lpool_t lpool_slabs;
/// Pula opisów buforów.
static vm_lpool_t lpool_bufctls;

static spinlock_t clock;

/*========================================================================
 * malloc
 */

/// Kube³ek.
typedef struct mem_bucket mem_bucket_t;
struct mem_bucket {
    size_t size;
    const char *name;
    kmem_cache_t *cache;
};

/// Obslugiwane kube³ki pamiêci.
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
    {0, NULL, NULL}
};

/**
 * Przydziela pamiêæ j±dra.
 * @param s wielko¶æ.
 * @param flags opcje przydzia³u.
 * @return wska¼nik do przydzielonego elementu.
 */
void*
kmem_alloc(size_t s, int flags)
{
    TRACE_IN0();
    int i;
    for (i = 0; buckets[i].size && buckets[i].size < s; i++);
    if (buckets[i].size == 0)
        panic("Allocating large regions not possible yet");
    return kmem_cache_alloc(buckets[i].cache, flags);
}

/// Inicjalizuje kube³ki.
void
alloc_init()
{
    TRACE_IN0();
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
 * @param esize wielko¶æ elementu.
 * @param ctor wska¼nik do konstruktora.
 * @param dtor wska¼nik do destruktora.
 */
kmem_cache_t*
kmem_cache_create(const char *name, size_t esize, kmem_ctor_t *ctor,
    kmem_dtor_t *dtor)
{
    spinlock_lock(&clock);
    kmem_cache_t *cache = vm_lpool_alloc(&lpool_caches);
    cache_init(cache);
    cache->elem_size = esize;
    cache->ctor = ctor;
    cache->dtor = dtor;
    spinlock_unlock(&clock);
    return cache;
}

/**
 * Niszczy kmem_cache.
 * @param cache wska¼nik do cache.
 */
void
kmem_cache_destroy(kmem_cache_t *cache)
{
    TRACE_IN0();
    spinlock_lock(&clock);
    cache_cleanup(cache);
    spinlock_lock(&clock);
}

/**
 * Przydziela element z danego kmem_cache.
 * @param cache wska¼nik do cache.
 * @param flags opcje przydzia³u.
 * @return wska¼nik do przydzielonego elementu.
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
    TRACE_IN0();
    mutex_lock(&cache->mtx);
    check_cache(cache);
    mutex_unlock(&cache->mtx);
}


/*========================================================================
 * maintain
 */

/// Inicjalizuje modu³.
void
kmem_init()
{
    TRACE_IN0();

    vm_lpool_create(&lpool_caches, offsetof(kmem_cache_t, L_caches), 
        sizeof(kmem_cache_t), VM_LPOOL_PREALLOC);
    vm_lpool_create(&lpool_slabs, offsetof(kmem_slab_t, L_slabs),
        sizeof(kmem_slab_t), VM_LPOOL_PREALLOC);
    vm_lpool_create(&lpool_bufctls, offsetof(kmem_bufctl_t, L_bufs),
        sizeof(kmem_bufctl_t), VM_LPOOL_PREALLOC);

    spinlock_init(&clock);
    alloc_init();
}


/*========================================================================
 * pomocnicze
 */

static void cache_init(kmem_cache_t *c);
static void cache_cleanup(kmem_cache_t *c);

/**
 * Inicjaluzuje schowek.
 * @param c wska¼nik do schowka
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
 * Sprz±ta po schowku.
 * @param c wska¼nik do schowka.
 */
void
cache_cleanup(kmem_cache_t *c)
{
    c->elem_size = 0;
    vm_lpool_free(&lpool_caches, c);
}

/**
 * Przygotowuje p³ytê dla danego schowka.
 * @param cache wska¼nik do schowka.
 * @param slab wska¼nik do p³yty.
 */
void
prepare_slab_for_cache(kmem_cache_t *cache, kmem_slab_t *slab)
{
    list_create(&slab->free_bufs, offsetof(kmem_bufctl_t, L_bufs), FALSE);
    list_create(&slab->used_bufs, offsetof(kmem_bufctl_t, L_bufs), FALSE);
    char *data = (char*)vm_segment_alloc(&vm_kspace.seg_data, PAGE_SIZE);
    KASSERT(data != NULL);
    if (cache->elem_size < LARGE_SIZE) {
        slab->addr = data;
        slab->items = PAGE_SIZE / ( sizeof(kmem_bufctl_t) + cache->elem_size );
        for (int i = 0; i < slab->items; i++) {
            kmem_bufctl_t *bctl = (kmem_bufctl_t*) data;
            bctl->addr = data + sizeof(kmem_bufctl_t);
            if (cache->ctor) cache->ctor(bctl->addr);
            list_insert_tail(&slab->free_bufs, bctl);
            data += cache->elem_size + sizeof(kmem_bufctl_t);
        }
    } else {
        panic("Allocating SLABS for LARGE not supported");
    }
}

/**
 * Pobiera u¿yteczn± p³ytê z schowka.
 * @param cache wska¼nik do schowka
 * @return wska¼nik do p³yty.
 *
 * Je¿eli nie ma u¿ytecznych p³yt w schowku to przydziela now±.
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
    kmem_slab_t *slab = vm_lpool_alloc(&lpool_slabs);
    prepare_slab_for_cache(cache, slab);
    return slab;
}

/**
 * Rezerwuje element z p³yty.
 * @param cache wska¼nik do schowka.
 * @param slan wska¼nik do u¿ytecznej p³yty.
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
 * Sprawdza poprawno¶æ schowka.
 * @param cache wska¼nik do schowka.
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
        vm_lpool_free(&lpool_slabs, slab);
    }
}

/*
 * Impala Operating System
 * http://trzask.codepainters.com/impala/trac/
 *
 */
#include <sys/types.h>
#include <sys/kmem.h>
#include <sys/vm.h>
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/kprintf.h>

typedef struct cache_slab cache_slab_t;
typedef struct slab_slab slab_slab_t;
typedef struct bufctl_slab bufctl_slab_t;

typedef struct kmem_slab kmem_slab_t;
typedef struct kmem_bufctl kmem_bufctl_t;

struct kmem_cache {
    size_t          elem_size;
    kmem_ctor_t     *ctor;
    kmem_dtor_t     *dtor;
    list_t          empty_slabs;
    list_t          part_slabs;
    list_t          full_slabs;
    mutex_t         mtx;
    list_node_t     L_caches;
    cache_slab_t   *cslab;
};

struct kmem_slab {
    list_t          free_bufs;
    list_t          used_bufs;
    size_t          items;
    void           *addr;
    list_node_t     L_slabs;
};


struct kmem_bufctl {
    kmem_slab_t     *slab;
    void            *addr;
    list_node_t     L_bufs;
};

enum {
    LARGE_SIZE  = PAGE_SIZE/4,
    CACHES_ON_PAGE = (PAGE_SIZE-sizeof(size_t))/sizeof(kmem_cache_t),
    SLABS_ON_PAGE = (PAGE_SIZE-sizeof(size_t))/sizeof(kmem_slab_t),
    BUFCTLS_ON_PAGE = (PAGE_SIZE-sizeof(size_t))/sizeof(kmem_bufctl_t)
};

struct cache_slab {
    size_t          used;
    kmem_cache_t    table[CACHES_ON_PAGE];
};

struct slab_slab {
    size_t          used;
    kmem_slab_t     table[SLABS_ON_PAGE];
};

struct bufctl_slab {
    size_t          used;
    kmem_bufctl_t   table[BUFCTLS_ON_PAGE];
};

static kmem_cache_t *alloc_new_cache(void);
static kmem_slab_t *alloc_new_slab(void);
static void create_new_caches(void);
static void create_new_slabs(void);
//static void create_new_bufctls(void);

static void cache_init(kmem_cache_t *c);
static void cache_cleanup(kmem_cache_t *c);
static void slab_init(kmem_cache_t *c, kmem_slab_t *s);
static void alloc_init(void);

static list_t free_caches;
static list_t free_slabs;
static list_t free_bufctls;

static spinlock_t clock;

/*========================================================================
 * malloc
 */
typedef struct mem_bucket mem_bucket_t;

struct mem_bucket {
    int size;
    const char *name;
    kmem_cache_t *cache;
};

static mem_bucket_t buckets[] = {
    {1 <<  1, "kmem_alloc bucket [2]", NULL},
    {1 <<  2, "kmem_alloc_bucket [4]", NULL},
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

void*
kmem_alloc(size_t s, int flags)
{
    TRACE_IN0();
    int i;
    for (i = 0; buckets[i].size && buckets[i].size < s; i++);
    if (buckets[i].size == 0)
        panic("Allocating large regions not possible");
    return kmem_cache_alloc(buckets[i].cache, flags);
}


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


kmem_cache_t*
kmem_cache_create(const char *name, size_t esize, kmem_ctor_t *ctor,
    kmem_dtor_t *dtor)
{
/*
    TRACE_IN("name=\"%s\" esize=%u ctor=%p dtor=%p", name, esize, ctor,
        dtor);
*/
    spinlock_lock(&clock);
    kmem_cache_t *c = alloc_new_cache();
    c->elem_size = esize;
    c->ctor = ctor;
    c->dtor = dtor;
    spinlock_unlock(&clock);
    return c;
}

void
kmem_cache_destroy(kmem_cache_t *cache)
{
    TRACE_IN0();
    spinlock_lock(&clock);
    cache_cleanup(cache);
    spinlock_lock(&clock);
}

void *
kmem_cache_alloc(kmem_cache_t *cache, int flags)
{
    TRACE_IN("cache=%p", cache);
    mutex_lock(&cache->mtx);
    kmem_slab_t *s;
    TRACE_IN("#empty=%u #part=%u #full=%u",
        list_length(&cache->empty_slabs),
        list_length(&cache->part_slabs),
        list_length(&cache->full_slabs));
    if (list_length(&cache->part_slabs) == 0) {
        if (list_length(&cache->empty_slabs) == 0) {
            s = alloc_new_slab();
            slab_init(cache, s);
        } else {
            s = list_extract_first(&cache->empty_slabs);
        }
    } else {
        s = list_extract_first(&cache->part_slabs);
    }
    TRACE_IN("slab=%p", s);   
    // Mamy gwarancje ze jest jakis bufctl, bo bralismy z part/empty
    kmem_bufctl_t *b = list_extract_first(&s->free_bufs);
    list_insert_tail(&s->used_bufs, b);
    if (list_length(&s->free_bufs) == 0) {
        list_insert_tail(&cache->full_slabs, s);
    } else {
        list_insert_tail(&cache->part_slabs, s);
    }
    mutex_unlock(&cache->mtx);
    return b->addr;
}

void
kmem_cache_free(kmem_cache_t *cache, void *m)
{
    TRACE_IN0();
    mutex_lock(&cache->mtx);
    mutex_unlock(&cache->mtx);
}




/*========================================================================
 * maintain
 */


void
kmem_init()
{
    TRACE_IN0();
    list_create(&free_caches, offsetof(kmem_cache_t, L_caches), FALSE);
    list_create(&free_slabs, offsetof(kmem_slab_t, L_slabs), FALSE);
    list_create(&free_bufctls, offsetof(kmem_bufctl_t, L_bufs), FALSE);
    spinlock_init(&clock);
    alloc_init();
}


/*========================================================================
 * pomocnicze
 */

static void cache_init(kmem_cache_t *c);
static void cache_cleanup(kmem_cache_t *c);

void
create_new_caches()
{
    TRACE_IN0();
    cache_slab_t *cslab = (cache_slab_t*)
        vm_segment_alloc(&vm_kspace.seg_data,sizeof(cache_slab_t));
    TRACE_IN("allocated at %p (%u)", cslab, sizeof(cache_slab_t));
    for (int i = 0; i < CACHES_ON_PAGE; i++) {
        list_insert_tail(&free_caches, &cslab->table[i]);
        cache_init(&cslab->table[i]);
        cslab->table[i].cslab = cslab;
    }
}

void
create_new_slabs()
{
    TRACE_IN0();
    slab_slab_t *sslab = (slab_slab_t*)
        vm_segment_alloc(&vm_kspace.seg_data,sizeof(slab_slab_t));
    TRACE_IN("allocated at %p (%u)", sslab, sizeof(slab_slab_t));
    for (int i = 0; i < SLABS_ON_PAGE; i++) {
        list_insert_tail(&free_slabs, &sslab->table[i]);
        list_create(&sslab->table[i].free_bufs, offsetof(kmem_bufctl_t,L_bufs),
            FALSE);
        list_create(&sslab->table[i].used_bufs, offsetof(kmem_bufctl_t,L_bufs),
            FALSE);
    }
}

void
cache_init(kmem_cache_t *c)
{
//    TRACE_IN0();
    list_create(&c->empty_slabs, offsetof(kmem_slab_t, L_slabs), FALSE);
    list_create(&c->part_slabs, offsetof(kmem_slab_t, L_slabs), FALSE);
    list_create(&c->full_slabs, offsetof(kmem_slab_t, L_slabs), FALSE);
    mutex_init(&c->mtx, MUTEX_CONDVAR);
    c->elem_size = 0;
}

void
cache_cleanup(kmem_cache_t *c)
{
//    TRACE_IN0();
    c->elem_size = 0;
    list_insert_head(&free_caches, c);
}

void
slab_init(kmem_cache_t *c, kmem_slab_t *s)
{
    TRACE_IN("cache=%p slab=%p", c, s);
    char *p = (char*)vm_segment_alloc(&vm_kspace.seg_data, PAGE_SIZE);
    if (c->elem_size < LARGE_SIZE) {
        s->addr = p; 
        s->items = PAGE_SIZE / (sizeof(kmem_bufctl_t) + c->elem_size);
        for (int i = 0; i < s->items; i++) {
            kmem_bufctl_t *b = (kmem_bufctl_t*) p;
            p += sizeof(kmem_bufctl_t);
            b->addr = p;
            p += c->elem_size;
            list_insert_tail(&s->free_bufs, b);
//            TRACE_IN("buf=%p data=%p", b, b->addr);
        }
    } else {
        panic("Allocating SLABS for LARGE not supported");
    }
}

kmem_cache_t *
alloc_new_cache()
{
    if (list_length(&free_caches) == 0) {
        create_new_caches();
    }
    kmem_cache_t *c = list_extract_first(&free_caches);
    return c;
}

kmem_slab_t *
alloc_new_slab()
{
    if (list_length(&free_slabs) == 0) {
        create_new_slabs();
    }
    kmem_slab_t *s = list_extract_first(&free_slabs);
    s->addr = NULL;
    return s;
}


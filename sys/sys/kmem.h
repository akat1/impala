#ifndef __SYS_KMEM_H
#define __SYS_KMEM_H

enum MALLOC_FLAGS {
    KM_SLEEP,
    KM_NOSLEEP
};

typedef void kmem_ctor_t(void *);
typedef void kmem_dtor_t(void *);


addr_t kmem_alloc(size_t s, int flags);
void kmem_free(void *a);

kmem_cache_t *kmem_cache_create(const char *name, size_t esize, 
        kmem_ctor_t *ctor, kmem_dtor_t *dtor);
void kmem_cache_destroy(kmem_cache_t *kc);

addr_t kmem_cache_alloc(kmem_cache_t *kc, int flags);
void kmem_cache_free(kmem_cache_t *kc, addr_t buf);


void kmem_init(void);

#endif

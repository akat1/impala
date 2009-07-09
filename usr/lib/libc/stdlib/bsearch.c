#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define BASE(X)         ((char *)base + size*(X))

void *
binary_search(const void *key, void *base, size_t size, 
        int (*compar)(const void *, const void *),int lo, int hi);

void *
binary_search(const void *key, void *base, size_t size, 
        int (*compar)(const void *, const void *),int lo, int hi)
{
    int mid = (lo+hi)/2;
    int result;

    result = compar(key, BASE(mid));

    if ( result == 0 ) {
        return BASE(mid);
    }

    if (lo == hi)
        return NULL;

    if ( result < 0 ) {
        return binary_search(key, base, size, compar, lo, mid);
     } else {
        return binary_search(key, base, size, compar, mid+1, hi);
    }
}


void *
bsearch(const void *key, void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *))
{
    return binary_search(key, base, size, compar, 0, nmemb-1); 
}

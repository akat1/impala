#include <string.h>
#include <sys/types.h>

void *
mem_move(void *dst, void *src, size_t len)
{
    void *org = dst;
    int i;

    if ( dst != src && len > 0 ) {
        if (dst < src) {
            for ( i = 0 ; i < len ; i++ )
                *((char *)dst+i) = *((char *)src+i);
        } else {
            for ( i = len - 1 ; i >= 0 ; i-- )
                *((char *)dst+i) = *((char *)src+i);
        }
    }
    return org;
}

#include <sys/types.h>
#include <string.h>

void *
memchr(const void *s, int c, size_t n)
{
    while(n--) {
        if ( *((char *)s) == (uchar)c ) {
            return (void *)s;
        }
        s++;
    }

    return NULL;
}

#include <string.h>
#include <sys/types.h>

void *
memchr(const void *s, int c, size_t n)
{
    while(n--) {
        if ( *s == (uchar)c ) {
            return s;
        }
        s++;
    }

    return NULL;
}

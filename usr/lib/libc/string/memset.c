#include <string.h>
#include <sys/types.h>

void *
memset(void *s, int c, size_t n)
{
    void *r = s;

    while(n--) {
        *((char *)s++) = (unsigned char)c;
    }

    return r;
}

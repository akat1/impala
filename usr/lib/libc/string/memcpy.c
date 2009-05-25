#include <sys/types.h>
#include <string.h>

void *
memcpy(void *dest, const void *src, size_t n)
{
    char *p1 = (char *)dest, *p2 = (char *)src;

    while(n--) {
        (*p1++) = (*p2++);
    }

    return dest;
}

#include <string.h>
#include <sys/types.h>

void *
memset(void *s, int c, size_t n)
{
    void *r = s;

    while(n--)
    {
        *(s++) = (unsigned char)c;
    }

    return s;
}

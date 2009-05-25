#include <string.h>
#include <sys/types.h>

char *
strncpy(char *dest, const char *src, size_t n)
{
    char *r = dest;

    while(n--) {
        if ( *src == '\0' ) {
            while(n--)
                *(dest++) = '\0';
            return r;
        }

        *(dest++) = *(src++);
    }

    return r;
}

#include <string.h>
#include <sys/types.h>

char *
strncat(char *dest, const char *src, size_t n)
{
    char *r = dest;

    while(*dest) {
        dest++;
    }

    while(n--) {
        if ( *src )
            break;

        *(dest++) = *(src++);
    }

    *dest = '\0';

    return r;
}

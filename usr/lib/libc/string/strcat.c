#include <string.h>
#include <sys/types.h>

char *
strcat(char *dest, const char *src)
{
    char *r = dest;

    while(*dest) { 
        dest++;
    }

    while(*src) {
        *(dest++) = *(src++);
    }

    *dest = '\0';

    return r;
}

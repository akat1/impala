#include <sys/types.h>
#include <string.h>

char *
strpbrk(const char *s, const char *accept)
{
    while(*s) {
        if ( strchr(accept, *s) != NULL ) {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

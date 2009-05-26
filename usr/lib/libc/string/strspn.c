#include <sys/types.h>
#include <string.h>

size_t 
strspn(const char *s, const char *accept)
{
    size_t r = 0;

    while(*s) {
        if ( strchr(accept, *s) == NULL ) {
            return r;
        }
        r++;
    }

    return r;
}

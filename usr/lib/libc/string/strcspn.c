#include <sys/types.h>
#include <string.h>

size_t 
strspn(const char *s, const char *reject)
{
    size_t r = 0;

    while(*s) {
        if ( strchr(reject, *s) != NULL ) {
            return r;
        }
        r++;
    }

    return r;
}

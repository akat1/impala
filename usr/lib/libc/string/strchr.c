#include <sys/types.h>
#include <string.h>
    
char *
strchr(const char *s, int c)
{
    for(; *s ; s++ ) {
        if ( *s == c ) {
            return (char *)s;
        }
    }

    return NULL;
}

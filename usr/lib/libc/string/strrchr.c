#include <string.h>
#include <sys/types.h>

char *
strrchr(const char *s, int c)
{
    char *last = NULL;

    for (; *s ; s++ ) {
        if ( *s == c ) {
            last = (char *)s;
        }
    }

    return last;
}

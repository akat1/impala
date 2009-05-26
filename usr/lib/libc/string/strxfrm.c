#include <string.h>
#include <sys/types.h>

size_t
strxfrm(char *dest, const char *src, size_t n)
{
    size_t r = n;
    
    while(n--) {
        if ( *src == '\0' ) {
            while(n--)
                *(dest++) = '\0';
            return r-n;
        }

        *(dest++) = *(src++);
    }

    return 0;
}

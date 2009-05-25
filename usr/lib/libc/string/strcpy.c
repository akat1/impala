#include <sys/types.h>
#include <string.h>
    
char *
strcpy(char *dest, const char *src)
{
    char *r = dest;

    while ((*src) != '\0')
        *(dest++) = *(src++);

    *dest = '\0';

    return r;
}

#include <string.h>
#include <sys/types.h>
    
int
memcmp(const void *s1, const void *s2, size_t n)
{
    uchar *p1 = (uchar *)s1, p2 = (uchar *)s2;
    
    while(n--)
    {
        if ( *s1 != *s2 ) {
            return *s1 - *s2;
        }

        s1++;
        s2++;
    }

    return 0;
}

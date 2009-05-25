#include <sys/types.h>
#include <string.h>
    
int
memcmp(const void *s1, const void *s2, size_t n)
{
    uchar *p1 = (uchar *)s1, *p2 = (uchar *)s2;
    
    while(n--) {
        if ( *p1 != *p2 ) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }

    return 0;
}

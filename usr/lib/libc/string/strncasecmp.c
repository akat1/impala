#include <string.h>
#include <sys/types.h>
#include <ctype.h>

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
    while(n--) {
        if ( tolower(*s1) != tolower(*s2) ) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }

    return 0;
}

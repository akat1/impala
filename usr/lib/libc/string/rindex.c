#include <sys/types.h>
#include <string.h>
#include <unistd.h>

char *
rindex(const char *s, int c)
{
    char *last = NULL;

    do 
    {
        if ( *s ==  (unsigned char)c )
            last = (char *)s;
    }
    while ( *(s++) );

    return last;
}

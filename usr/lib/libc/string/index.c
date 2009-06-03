#include <sys/types.h>
#include <string.h>
#include <unistd.h>

char *
index(const char *s, int c)
{

    do 
    {
        if ( *s ==  (unsigned char)c )
            return (char *)s;
    }
    while ( *(s++) );

    return NULL;
}

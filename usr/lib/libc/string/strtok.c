#include <sys/types.h>
#include <string.h>
    
char *
strtok(char *s, const char *delim)
{
    static char *w;
    char *r;
    int delim_size;

    if ( s != NULL )
        w = s;

    if ( *w == '\0' )
        return NULL;

    r = w;

    delim_size = strlen(delim);

    while ( *w != '\0' ) {
        for ( int i = 0 ; i < delim_size ; i++ ) {
            if ( *w == delim[i] ) {
                *w = '\0';
                w++;
                return r;
            }
        }

        w++;
    }

    return r;

}

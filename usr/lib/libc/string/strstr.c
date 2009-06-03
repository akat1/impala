#include <sys/types.h>
#include <string.h>
#include <unistd.h>

/* KMP */

char *
strstr(const char *haystack, const char *needle)
{
    int len_needle, len_haystack;
    char *n;
    int *P;
    int i,j;
    int q;
    char *ret;

    ret = NULL;

    len_needle = strlen(needle);
    len_haystack = strlen(haystack);

    n = (char *)malloc(sizeof(char)*(len_needle+1));
    P = (int *)malloc(sizeof(long int)*(len_needle+1));

    P[0] = -1;

    /* KMP - prefix */
    for ( i = 0 , j = -1 ; i < len_needle ; i++, j++, P[i] = j ) {
        while ( j >= 0 && (needle[i]!=needle[j]) ) j=P[j];
    }

    q=0; // gdzie siê dopasowali¶my
    
    for ( i = 0 ; i < len_haystack ; ++i ) {
        while( q >= 0 && ( needle[q+1] != haystack[i] )) q = P[q];
        if ( needle[q+1] == haystack[i] ) q++;
        if ( q == len_needle-1 ) {
            ret = (char *)&haystack[i-len_needle];
            break; /* dopasowali¶my siê */
        }
    }
   
    free(P);
    free(n);
    return ret;
}

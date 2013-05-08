/*
 * System operacyjny Impala.
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
 * All rights reserved.
 *
 * Niniejszy plik jest objęty licencją, zobacz plik COPYRIGHT dostarczony
 * wraz z projektem.
 *
 * $Id$
 */ 
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

    q=0; // gdzie się dopasowaliśmy
    
    for ( i = 0 ; i < len_haystack ; ++i ) {
        while( q >= 0 && ( needle[q+1] != haystack[i] )) q = P[q];
        if ( needle[q+1] == haystack[i] ) q++;
        if ( q == len_needle-1 ) {
            ret = (char *)&haystack[i-len_needle];
            break; /* dopasowaliśmy się */
        }
    }
   
    free(P);
    free(n);
    return ret;
}

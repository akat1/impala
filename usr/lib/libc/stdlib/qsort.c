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
#include <stdlib.h>

#define BASE(X)         ((char *)base + size*(X))
#define TMP(X)          ((char *)tmp + size*(X))

void swap(void *x,void *y, size_t size);
void quick_sort_split(void *base, int start, int stop, size_t size,
                int (*compar)(const void *, const void *));

void swap(void *x,void *y, size_t size)
{
        char *tmp = (char *)malloc(size);

        memcpy(tmp, x, size);
        memcpy(x, y, size);
        memcpy(y, tmp, size);
}

void quick_sort_split(void *base, int start, int stop, size_t size,
                int (*compar)(const void *, const void *))
{
        char *med = BASE(start);
        int left=start+1, right=stop;

        while(left <= right) {
                while ( compar(BASE(left),med) < 0 && left <= right ) {
                        left++;
		}
                while ( compar(BASE(right),med) >= 0 && left <= right ) {
                        right--;
		}
                if ( left < right ) {
                        swap(BASE(left),BASE(right),size);
		}
        }

        swap(BASE(right),med,size);

        if ( right > start ) {
                quick_sort_split(base, start, right-1, size, compar);
	}
        if ( left < stop ) {
                quick_sort_split(base, left, stop, size, compar);
	}

        return;
}

void qsort(void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *))
{
        quick_sort_split(base, 0, nmemb-1, size, compar);

        return;
}


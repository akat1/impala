#include <sys/types.h>
#include <sys/string.h>

/*
 * mem_move()
 * kopiuje len bajtow z src do dst
 */

addr_t
mem_move(addr_t dst, addr_t src, size_t len)
{
    size_t i;

    if ( dst != src && len > 0 ) {        
        if (dst < src) {
            for ( i = 0 ; i < len ; i++ )
                *((char *)dst+i) = *((char *)src+i);
        } else {
            for ( i = len - 1 ; i >= 0 ; i-- )
                *((char *)dst+i) = *((char *)src+i);
        }
    }
    return dst;
}

/*
 * mem_cpy()
 * kopiuje len bajtow z src do dst
 */

addr_t
mem_cpy(addr_t _dst, addr_t _src, size_t len)
{
    char *dst = (char*)_dst;
    char *src = (char*)_src;
    for (; len > 0; src++,dst++, len--) {
        *dst = *src;
    }
    return dst;
}

/*
 * mem_set()
 * ustawia len bajtow poczawszy od adresu s na c
 */

addr_t
mem_set(addr_t s, char c, size_t len)
{
    while(len > 0) {
        len--;
        *((char *)s+len) = c;
    }
    return s;

}

/*
 * str_len()
 * zwraca dlugosc napisu s nie liczac znaku '\0'
 */

size_t
str_len(const char *s)
{
    size_t len;
    for (len = 0; *s != 0; s++, len++);
    return len;
}



#include <stdlib.h>
#include <string.h>
#include <ctype.h>

long
atol(const char *nptr)
{
    long res = 0, sign;
    while(isspace(*nptr))
        nptr++;
    if(*nptr == '-') {
        nptr++;
        sign = -1;
    } else {
        if(*nptr == '+')
            nptr++;
        sign = 1;
    }
    while(*nptr) {
        if(isdigit(*nptr))
            res = res*10 + *nptr-'0';
        else break;
        nptr++;
    }
    return sign*res;
}

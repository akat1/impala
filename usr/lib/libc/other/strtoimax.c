#include <sys/types.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>

intmax_t
strtoimax(const char *nptr, char **endptr, int base)
{
    if(endptr)
        *endptr = (char*)nptr;
    if(base<0 || base == 1 || base > 36) {
        errno = EINVAL;
        return 0;
    }
    while(isspace(*nptr))
        nptr++;
    intmax_t res = 0, sign;
    if(*nptr == '-') {
        nptr++;
        sign = -1;
    } else {
        if(*nptr == '+')
            nptr++;
        sign = 1;
    }
    if(base == 0) {
        if(*nptr == '0') {
            if(*(nptr+1) == 'x') {
                base = 16;
                nptr+=2;
            }
            else
                base = 8;
        } else base = 10;
    }
    while(*nptr) {
        char c = *nptr;
        c = tolower(c);
        uint8_t val = 50;
        if(isalpha(c))
            val = c-'a';
        else if(isdigit(c))
            val = c-'0';
        if(val >= base || val<0)
            break;
        res = res*base + val;
        nptr++;
    }
    errno = 0;
    if(endptr)
        *endptr = (char*)nptr;
    
    return sign * res;
}

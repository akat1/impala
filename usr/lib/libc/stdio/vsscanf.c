#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

static int _parse_int(const char **src);
static void _parse_str(const char **src, char *res);

void
_parse_str(const char **src, char *res)
{
    while(!isspace(**src)) {
        *res = **src;
        (*src)++;
        res++;
    }
    *res = '\0';
}

int
_parse_int(const char **src)
{
    int res=0;
    
    while(isdigit(**src)) {
        res = 10*res + (**src - '0');
        (*src)++;
    }
    return res;
}

int
vsscanf(const char *src, const char *fmt, va_list ap)
{
    int res=0;
    const char *f = fmt;
    const char *s = src;
    while(*f) {
        if(*s == '\0')
            break;
        else if(*f == '%') {//jazda!
            f++;
            switch(*f) {
                case 's': {
                    char *sptr = va_arg(ap, char*);
                    _parse_str(&s, sptr);
                    res++;
                    break;
                }
                case 'd': {
                    int *iptr = va_arg(ap, int*);
                    *iptr = _parse_int(&s);
                    res++;
                    break;
                }
                case 'c': {
                    char *cptr = va_arg(ap, char*);
                    *cptr = *s;
                    res++;
                    f++;
                    s++;
                    break;
                }
            }
        } else if(*f == *s) {
            //ok, zmatchowa³o siê
            f++;
            s++;
            continue;
        } else {
            if(!res)
                res = EOF;
            break;
        }
    }
    return res;
}

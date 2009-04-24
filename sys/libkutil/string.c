/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

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
    ssize_t len;
    for (len = 0; *s != 0; s++, len++);
    return len;
}


int
str_cmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;

}

///////////////////////////////////////////////////////



static char *convert_uint32(char *b, uint32_t arg_u32);
static char *convert_hexuint32(char *b, uint32_t arg_u32);
static char *convert_binuint32(char *b, uint32_t arg_u32);
static int from_string(char *dst, int *i, int *left, char *b, char sep, int fw, 
                        bool to_right);

#define DEFAULT_ATTRIBUTE (COLOR_WHITE)



enum {
    INTERNAL_BUF = 128
};

int
snprintf(char *dst, size_t size, const char *fmt, ...)
{
    va_list ap;
    VA_START(ap, fmt);
    int res = vsnprintf(dst, size, fmt, ap);
    VA_END(ap);
    return res;
}

int
vsnprintf(char *dst, size_t size, const char *fmt, va_list ap)
{
    int i;
    int left = size - 1;
    char buf[INTERNAL_BUF];
    char *pbuf;
    char cbuf[2];
    uint32_t arg_u32;
    if(!dst || !fmt)
        return -1;  // -EBLEBLE ?
    
    for (i = 0; *fmt; i++, fmt++) {
        pbuf = 0;
        switch (*fmt) {
            case '%': {
                fmt++;
                int field_width = 0;
                char separator = ' ';
                bool pad_to_right = TRUE;
                bool done = FALSE;
                // zjedz flagi
                while(!done) {
                    switch (*fmt) {
                    case '-':
                        pad_to_right = FALSE;
                        fmt++;
                        break;
                    case '.':
                    case '0':
                        if(field_width == 0)
                            separator = '0';
                        else 
                            field_width *= 10;
                        fmt++;
                        break;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        field_width = 10*field_width + (*(fmt++) - '0');
                        break;
                    default:
                        done=TRUE;
                        break;
                    }
                }
                if(!pad_to_right)
                    separator = ' ';
                // flagi zjedzone - do roboty
                switch (*fmt) {
                    case '%':
                        if(left-- > 0)
                            *(dst++)='%';
                        break;
                    case 'u':
                        arg_u32 = VA_ARG(ap, uint32_t);
                        pbuf = convert_uint32(buf, arg_u32);
                        break;
                    case 'x':
                        arg_u32 = VA_ARG(ap, uint32_t);
                        pbuf = convert_hexuint32(buf, arg_u32);
                        break;
                    case 'b':
                        arg_u32 = VA_ARG(ap, uint32_t);
                        pbuf = convert_binuint32(buf, arg_u32);
                        break;
                    case 'p':
                        arg_u32 = VA_ARG(ap, uintptr_t);
                        pbuf = convert_hexuint32(buf, arg_u32);
                        *(--pbuf) = 'x';
                        *(--pbuf) = '0';
                        break;
                    case 's':
                        pbuf = VA_ARG(ap, char *);
                        break;
                    case 'c':
                        cbuf[0] = VA_ARG(ap, uint32_t);
                        cbuf[1] = 0;
                        pbuf = cbuf;
                        break;
                }
                if (pbuf)
                    dst += from_string(dst, &i, &left, pbuf, separator,
                                        field_width, pad_to_right);
                break; 
            }

            default:
                if(left-- > 0)
                    *(dst++) = *fmt;
                break;
        }
    }
    *dst = 0;

    return i;
}

int
from_string(char *dst, int *i, int *left, char *b, char sep, int fw, 
                        bool to_right)
{
    char *dst_orig=dst;
    int len = str_len(b);
    if(fw < len)
        fw = len;
    int pad_count = fw - len;
    
    if(to_right) {
        while(pad_count--) 
            if((*left)-- > 0)
                *(dst++) = sep;
    }
    while (*b != 0)
        if((*left)-- > 0)
            *(dst++) = *(b++);
    
    while(pad_count-- > 0) 
        if((*left)-- > 0)
            *(dst++) = sep;
    
    *i += fw-1;
    return dst-dst_orig;
}

char *
convert_uint32(char *buf, uint32_t arg)
{
    buf += INTERNAL_BUF -1;
    *buf = 0;
    buf[-1] = '0';

    /* jezeli argument jest zerem to wychodzimy */
    if ( arg == 0 )
        return buf-1;

    while (arg>0) {
        buf--;
        *buf = '0' + arg % 10;
        arg /= 10;
    }
    return buf;
}

char *
convert_hexuint32(char *buf, uint32_t arg)
{
    char digits[] = "0123456789abcdef";
    buf += INTERNAL_BUF -1;
    *buf = 0;
    buf[-1] = '0';

    /* jezeli argument jest zerem to wychodzimy */
    if ( arg == 0 )
        return buf-1;

    while (arg>0) {
        buf--;
        *buf = digits[arg % 0x10];
        arg /= 0x10;
    }
    return buf;
}

char *
convert_binuint32(char *buf, uint32_t arg)
{
    buf += INTERNAL_BUF -1;
    *buf = 0;
    buf[-1] = '0';
    /* jezeli argument jest zerem to wychodzimy */
    if ( arg == 0 )
        return buf-1;

    while (arg>0) {
        buf--;
        *buf = '0' + (arg % 2);
        arg /= 2;
    }
    return buf;
}


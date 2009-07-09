#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


static char *convert_int32(char *b, int32_t arg_u32, int base);
static char *convert_uint32(char *b, uint32_t arg_u32, int base);
static int from_string(char *dst, int *left, char *b, char sep, int fw, int fp,
                        bool to_right);
                        

enum {
    INTERNAL_BUF = 128
};


int
vsnprintf(char *dst, size_t size, const char *fmt, va_list ap)
{
    char *dstorg = dst;
    int left = size - 1;
    char buf[INTERNAL_BUF];
    char *pbuf;
    uint32_t arg_u32;
    if(!dst || !fmt) {
        errno = EINVAL;
        return -1;
    }

    for (; *fmt; fmt++) {
        pbuf = 0;
        switch (*fmt) {
            case '%': {
                fmt++;
                int field_width = 0;
                int field_precision = -1;
                int *val = &field_width;
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
                    case '*':
                        *val = va_arg(ap, int);
                        fmt++;
                        if(*val<0) {
                            *val=-*val;
                            pad_to_right = FALSE; //?
                        }
                        break;
                    case '.':
                        val = &field_precision;
                        *val = 0;
                        fmt++;
                        break;
                    case '0':
                        if(*val == 0)
                            separator = '0';
                        else
                            *val *= 10;
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
                        *val = 10*(*val) + (*(fmt++) - '0');
                        break;
                    default:
                        done=TRUE;
                        break;
                    }
                }
                if(!pad_to_right)
                    separator = ' ';
                if(*fmt == 'l') //long
                    fmt++;
                // flagi zjedzone - do roboty
                bool string = FALSE;
                switch (*fmt) {
                    case '%':
                        if(left-- > 0)
                            *(dst++)='%';
                        break;
                    case 'u':
                        arg_u32 = va_arg(ap, uint32_t);
                        pbuf = convert_uint32(buf, arg_u32, 10);
                        break;
                    case 'd':
                    case 'i':
                        arg_u32 = va_arg(ap, int32_t);
                        pbuf = convert_int32(buf, (int32_t)arg_u32, 10);
                        break;
                    case 'x':
                        arg_u32 = va_arg(ap, uint32_t);
                        pbuf = convert_uint32(buf, arg_u32, 16);
                        break;
                    case 'o':
                        arg_u32 = va_arg(ap, uint32_t);
                        pbuf = convert_uint32(buf, arg_u32, 8);
                        break;
                    case 'b':
                        arg_u32 = va_arg(ap, uint32_t);
                        pbuf = convert_uint32(buf, arg_u32, 2);
                        break;
                    case 'p':
                        arg_u32 = va_arg(ap, uintptr_t);
                        pbuf = convert_uint32(buf, arg_u32, 16);
                        *(--pbuf) = 'x';
                        *(--pbuf) = '0';
                        break;
                    case 's':
                        pbuf = va_arg(ap, char *);
                        string = TRUE;
                        break;
                    case 'c':
                        if(left-- > 0)
                            *(dst++)=(unsigned char)va_arg(ap, uint32_t);
                        break;
                }
                if (pbuf)
                    dst += from_string(dst, &left, pbuf, separator, field_width,
                                       string?field_precision:-1, pad_to_right);
                break;
            }

            default:
                if(left-- > 0)
                    *(dst++) = *fmt;
                break;
        }
    }
    *dst = 0;

    return ((uintptr_t)dst-(uintptr_t)dstorg);
}

int
from_string(char *dst, int *left, char *b, char sep, int fw, int fp,
                        bool to_right)
{
    char *dst_orig=dst;
    int len = strlen(b);
    if(fw < len)
        fw = len;
    int pad_count = fw - len;

    if(to_right) {
        while(pad_count--)
            if((*left)-- > 0)
                *(dst++) = sep;
    }
    if(fp == -1) {
        while (*b != 0)
            if((*left)-- > 0)
                *(dst++) = *(b++);
    } else {
        while (*b != 0)
            if((*left)-- > 0 && fp-- > 0)
                *(dst++) = *(b++);
    }

    while(pad_count-- > 0)
        if((*left)-- > 0)
            *(dst++) = sep;

    return dst-dst_orig;
}

char *
convert_int32(char *buf, int32_t arg, int base)
{
    char digits[] = "0123456789abcdef";
    bool min = FALSE;
    buf += INTERNAL_BUF -1;
    *buf = 0;
    buf[-1] = '0';
    /* jezeli argument jest zerem to wychodzimy */
    if ( arg == 0 )
        return buf-1;
    if ( arg < 0 ) {
        min = TRUE;
        arg = -arg;
    }

    while (arg>0) {
        buf--;
        *buf = digits[arg % base];
        arg /= base;
    }
    if(min) {
        *(--buf) = '-';
    }
    return buf;
}



char *
convert_uint32(char *buf, uint32_t arg, int base)
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
        *buf = digits[arg % base];
        arg /= base;
    }
    return buf;
}


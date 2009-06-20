#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdio_private.h>
#include <string.h>
#include <stdarg.h>


static char *convert_int32(char *b, int32_t arg_u32);
static char *convert_uint32(char *b, uint32_t arg_u32);
static char *convert_hexuint32(char *b, uint32_t arg_u32);
static char *convert_binuint32(char *b, uint32_t arg_u32);
static int from_string(FILE *str, char *b, char sep, int fw, bool to_right);

enum {
    INTERNAL_BUF = 128
};


int
vfprintf(FILE *stream, const char *fmt, va_list ap)
{
    if(ISUNSET(stream->status,_FST_OPEN))
        return -1;
    __check_buf(stream);
    int tot = 0;
    char buf[INTERNAL_BUF];
    char *pbuf;
    char cbuf[2];
    uint32_t arg_u32;
    if(!stream || !fmt) {
        errno = EINVAL;
        return -1;
    }

    for (; *fmt; fmt++) {
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
                        __put_char(stream, '%');
                        tot++;
                        break;
                    case 'u':
                        arg_u32 = va_arg(ap, uint32_t);
                        pbuf = convert_uint32(buf, arg_u32);
                        break;
                    case 'd':
                    case 'i':
                        arg_u32 = va_arg(ap, int32_t);
                        pbuf = convert_int32(buf, (int32_t)arg_u32);
                        break;
                    case 'x':
                        arg_u32 = va_arg(ap, uint32_t);
                        pbuf = convert_hexuint32(buf, arg_u32);
                        break;
                    case 'b':
                        arg_u32 = va_arg(ap, uint32_t);
                        pbuf = convert_binuint32(buf, arg_u32);
                        break;
                    case 'p':
                        arg_u32 = va_arg(ap, uintptr_t);
                        pbuf = convert_hexuint32(buf, arg_u32);
                        *(--pbuf) = 'x';
                        *(--pbuf) = '0';
                        break;
                    case 's':
                        pbuf = va_arg(ap, char *);
                        break;
                    case 'c':
                        cbuf[0] = va_arg(ap, uint32_t);
                        cbuf[1] = 0;
                        pbuf = cbuf;
                        break;
                }
                if (pbuf)
                    tot+=from_string(stream, pbuf, separator,
                                        field_width, pad_to_right);
                break;
            }

            default:
                __put_char(stream, *fmt);
                break;
        }
    }

    return tot;
}

int
from_string(FILE *str, char *b, char sep, int fw,
                        bool to_right)
{
    int len = strlen(b);
    if(fw < len)
        fw = len;
    int pad_count = fw - len;

    if(to_right) {
        while(pad_count--)
            __put_char(str, sep);
    }
    __put_str(str, b);

    while(pad_count-- > 0)
        __put_char(str, sep);

    return fw;
}

char *
convert_int32(char *buf, int32_t arg)
{
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
        *buf = '0' + arg % 10;
        arg /= 10;
    }
    if(min) {
        *(--buf) = '-';
    }
    return buf;
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


#include <sys/types.h>
#include <sys/kprintf.h>
#include <machine/video.h>

static char *convert_uint32(char *b, uint32_t arg_u32);
static char *convert_hexuint32(char *b, uint32_t arg_u32);
static char *convert_binuint32(char *b, uint32_t arg_u32);
static int from_string(int i, char *b);

#define DEFAULT_ATTRIBUTE (COLOR_WHITE)



enum {
    INTERNAL_BUF = 128
};

void
kprintf(const char *fmt, ...)
{
    va_list ap;
    VA_START(ap, fmt);
    vkprintf(fmt, ap);
    VA_END(ap);
}

void
vkprintf(const char *fmt, va_list ap)
{
    int i;
    char buf[INTERNAL_BUF];
    char *pbuf;
    char cbuf[2];
    uint32_t arg_u32;
    
    for (i = 0; *fmt; i++, fmt++) {
        pbuf = 0;
        switch (*fmt) {
            case '%':
                fmt++;
                switch (*fmt) {
                    case '%':
                        textscreen_put(&textscreen, '%', DEFAULT_ATTRIBUTE);
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
                    i = from_string(i, pbuf);
                break; 

            case '\n':
                if ( textscreen.cursor_y < TEXTSCREEN_HEIGHT-1 )
                    textscreen_update_cursor(&textscreen, 0, 
                        textscreen.cursor_y+1);
                else
                    textscreen_scroll(&textscreen);

                break;

            default:
                textscreen_put(&textscreen, *fmt, DEFAULT_ATTRIBUTE);
                break;
        }
    }

    textscreen_draw(&textscreen);
    
    return;
}

int
from_string(int i, char *b)
{
    while (*b != 0) {
        textscreen_put(&textscreen, *b, DEFAULT_ATTRIBUTE);
        b++;
    }
    i--;
    return i;
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

/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/string.h>
#include <sys/kprintf.h>
#include <machine/video.h>

#define DEFAULT_ATTRIBUTE (COLOR_WHITE)



enum {
    KPRINTF_BUF = 2048
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
    char big_buf[KPRINTF_BUF];
    char *ptr=big_buf;
    mem_zero(big_buf, KPRINTF_BUF);
    vsnprintf(big_buf, KPRINTF_BUF, fmt, ap);
    
    for (; *ptr; ptr++) {
        switch (*ptr) {
            case '\n':
                textscreen_next_line(NULL);
                break;
            case '\t':
                for(int j=0; j<6; j++)
                    textscreen_put(NULL, ' ', DEFAULT_ATTRIBUTE);
                break;

            default:
                textscreen_put(NULL, *ptr, DEFAULT_ATTRIBUTE);
                break;
        }
    }

//   textscreen_draw(NULL);
}

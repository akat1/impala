#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int
sscanf(const char *src, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int res = vsscanf(src, fmt, ap);
    va_end(ap);
    return res;
}

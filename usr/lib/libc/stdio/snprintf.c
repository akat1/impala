#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>


int
snprintf(char *dst, size_t size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int res = vsnprintf(dst, size, fmt, ap);
    va_end(ap);
    return res;
}
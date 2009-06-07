#include <sys/types.h>
#include <stdio.h>


int
snprintf(char *dst, size_t size, const char *fmt, ...)
{
    va_list ap;
    VA_START(ap, fmt);
    int res = vsnprintf(dst, size, fmt, ap);
    VA_END(ap);
    return res;
}
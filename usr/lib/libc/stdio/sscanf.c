#include <stdio.h>
#include <string.h>

int
sscanf(const char *src, const char *fmt, ...)
{
    va_list ap;
    VA_START(ap, fmt);
    int res = vsscanf(src, fmt, ap);
    VA_END(ap);
}

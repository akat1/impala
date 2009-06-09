#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>

int
sprintf(char *str, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = vsnprintf(str, 1024, format, ap);
    va_end(ap);
    return res;
}

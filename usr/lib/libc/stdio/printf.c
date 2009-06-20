#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int
printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = vfprintf(stdout, format, ap);
    va_end(ap);
    return res;
}

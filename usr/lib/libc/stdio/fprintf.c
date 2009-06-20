#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int
fprintf(FILE *f, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = vfprintf(f, format, ap);
    va_end(ap);
    return res;
}

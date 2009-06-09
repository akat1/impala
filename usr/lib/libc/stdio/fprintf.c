#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int
fprintf(FILE *f, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buf[1024];
    vsnprintf(buf, 1024, format, ap);
    va_end(ap);
    return fwrite(buf, strlen(buf), 1, f);
}

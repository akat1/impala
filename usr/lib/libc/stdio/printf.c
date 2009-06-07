#include <sys/types.h>
#include <stdio.h>
#include <string.h>

int
printf(const char *format, ...)
{
    va_list ap;
    VA_START(ap, format);
    char buf[1024];
    vsnprintf(buf, 1024, format, ap);
    VA_END(ap);
    return fwrite(buf, strlen(buf), 1, stdout);
}

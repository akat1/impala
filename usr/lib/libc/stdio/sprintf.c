#include <sys/types.h>
#include <stdio.h>

int
sprintf(char *str, const char *format, ...)
{
    va_list ap;
    VA_START(ap, format);
    return vsnprintf(str, 1024, format, ap);
    VA_END(ap);
}

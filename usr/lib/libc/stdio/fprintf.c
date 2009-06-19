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
    if(f->writefn)
        return f->writefn(f->cookie, buf, strlen(buf));
    if(f->fd != -1)
        return fwrite(buf, strlen(buf), 1, f);
    return -1;
}

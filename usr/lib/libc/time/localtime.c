#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>

struct tm *
localtime(const time_t *t)
{
    static struct tm globalbuf;
    return localtime_r(t, &globalbuf);
}

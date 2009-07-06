#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

time_t
time(time_t *t)
{
    struct timeval tv;
    int err = gettimeofday(&tv, NULL);
    if (err) return err;
    if (t) *t = tv.tv_sec;
    return tv.tv_sec;
}

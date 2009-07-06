#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <time.h>

static int months[2][12] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static int years[] = {
    365, 366
};

#define LEAP(y) (y)%4
struct tm *
localtime_r(const time_t *t, struct tm *buf)
{
    enum {
        UNIX_EPOCH = 1970,
    };
    memset(buf, 0, sizeof(*buf));
	return 0;
}

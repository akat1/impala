#ifndef __UTIME_H__
#define __UTIME_H__

#include <sys/time.h>

struct utimbuf {
    time_t actime;
    time_t modtime;
};

int utime(const char *filename, const struct utimbuf *times);




#endif

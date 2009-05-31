#ifndef __SYS_RESOURCE_H
#define __SYS_RESOURCE_H

#include <sys/time.h>

struct rusage {
    timeval_t ru_utime;
    timeval_t ru_stime;
};

#endif

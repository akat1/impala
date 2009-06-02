#ifndef __SYS_RESOURCE_H
#define __SYS_RESOURCE_H

#include <sys/time.h>

typedef unsigned int rlim_t;

struct rusage {
    timeval_t ru_utime;
    timeval_t ru_stime;
};

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};


#define RLIM_INFINITY 1000000000

#define RUSAGE_SELF     0
#define RUSAGE_CHILDREN 1

#ifdef __KERNEL
#else

// int  getpriority(int, id_t);
int  getrlimit(int, struct rlimit *);
int  getrusage(int, struct rusage *);
// int  setpriority(int, id_t, int);
int  setrlimit(int, const struct rlimit *);


#endif


#endif

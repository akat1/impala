#ifndef __SYS_WAIT_H
#define __SYS_WAIT_H


//dla waitpid
#define   WNOHANG         1<<0
#define   WUNTRACED       1<<1


#ifdef __KERNEL
#else

#include <sys/resource.h>

pid_t wait3(int *status, int opcje, struct rusage *ruzycie);



#endif

#endif

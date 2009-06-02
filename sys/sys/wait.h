#ifndef __SYS_WAIT_H
#define __SYS_WAIT_H


//dla waitpid
#define   WNOHANG         1<<0
#define   WUNTRACED       1<<1



///@todo wype³niæ
#define WIFSTOPPED(st) 1
#define WIFSIGNALED(st) 1
#define WIFEXITED(st) 1
#define WEXITSTATUS(st) 1
#define WTERMSIG(st) 1
#define WCOREDUMP(st) 1
#define WSTOPSIG(st) st

#ifdef __KERNEL
#else

#include <sys/resource.h>

pid_t wait3(int *status, int opcje, struct rusage *ruzycie);



#endif

#endif

#ifndef __SYS_WAIT_H
#define __SYS_WAIT_H


//dla waitpid
#define   WNOHANG         1<<0
#define   WUNTRACED       1<<1

#define __PE_NORMAL    (0<<8)
#define __PE_SIGNALED  (1<<8)
#define __PE_STOPPED   (2<<8)

#define WIFSTOPPED(st) ((st>>8) == __PE_STOPPED)
#define WIFSIGNALED(st) ((st>>8) == __PE_SIGNALED)
#define WIFEXITED(st)   ((st>>8) == __PE_NORMAL)
#define WEXITSTATUS(st) (st&0xff)
#define WTERMSIG(st)    (st&0xff)
#define WCOREDUMP(st)   0
#define WSTOPSIG(st)    (st&0xff)

#ifdef __KERNEL



#define MAKE_STATUS_EXITED(code) ((code&0xff) | __PE_NORMAL)
#define MAKE_STATUS_STOPPED(code) ((code&0xff) | __PE_STOPPED)
#define MAKE_STATUS_SIGNALED(code) ((code&0xff) | __PE_SIGNALED)

#else

#include <sys/resource.h>

pid_t wait3(int *status, int options, struct rusage *rusage);
pid_t waitpid(pid_t pid, int *status, int options); 


#endif

#endif

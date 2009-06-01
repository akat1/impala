#ifndef __SYS_SELECT_H
#define __SYS_SELECT_H

#include <sys/time.h>

#ifdef __KERNEL
#else

#define FD_SETSIZE 128

typedef int fd_set[FD_SETSIZE / sizeof(int)];

int select(int nfds, fd_set *readfds, fd_set *writefds,
            fd_set *exceptfds, struct timeval *timeout);



#define FD_CLR(fd, set) do { (*set)[(fd)/32] &= ~(1<<((fd)%32)); } while(0)
#define FD_ISSET(fd, set)   ((*set)[(fd)/32] & (1<<((fd)%32)))
#define FD_SET(fd, set) do { (*set)[(fd)/32] |= (1<<((fd)%32)); } while(0)
#define FD_ZERO(set)    do { memset((*set), 0, sizeof(*set)); } while(0)

#endif


#endif

#ifndef __SYS_IOCTL_H
#define __SYS_IOCTL_H

//tymczasowo tutaj:
#define TIOCSPGRP 1
#define TIOCGPGRP 2
#define TCGETS 3
#define TCSETS 4

#ifdef __KERNEL
#else
int ioctl(int fd, int request, ...);
#endif

#endif

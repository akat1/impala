#ifndef __SYS_IOCTL_H
#define __SYS_IOCTL_H

//tymczasowo tutaj:
//#define TIOCSPGRP 1
//#define TIOCGPGRP 2
#define TIOCGETS 3

#ifdef __KERNEL
#else
int ioctl(int fd, unsigned long int request, ...);
#endif

#endif

#ifndef __SYS_IOCTL_H
#define __SYS_IOCTL_H

//tymaczosowo tutaj:
#define TIOCSPGRP 1
#define TIOCGPGRP 2

#ifdef __KERNEL
#else
int ioctl(int fd, unsigned long int request, ...);
#endif

#endif

#ifndef __SIGNAL_H
#define __SIGNAL_H

#include <sys/stat.h>
#include <sys/signum.h>
#include <sys/wait.h> //tak na razie...

#define NSIG _NSIG

extern const char *const sys_siglist[NSIG]; //niby stare, ale ash chce

int kill(pid_t pid, int sig);
int killpg(int pgrp, int sig);


#endif

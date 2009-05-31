#ifndef __SIGNAL_H
#define __SIGNAL_H

#include <sys/stat.h>
#include <sys/signum.h>
#include <sys/wait.h> //tak na razie...

#define NSIG _NSIG

extern const char *const sys_siglist[NSIG]; //niby stare, ale ash chce

typedef int sig_atomic_t;

int kill(pid_t pid, int sig);
int killpg(int pgrp, int sig);



// tak narazie
typedef int sigset_t; 
#define SIG_DFL 0
#define SIG_SETMASK 0
#define WIFSIGNALED(X) (X)

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);
int sigemptyset(sigset_t *set);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

#endif

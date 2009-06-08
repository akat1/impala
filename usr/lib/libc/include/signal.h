#ifndef __SIGNAL_H
#define __SIGNAL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signum.h>
#include <sys/wait.h> //tak na razie...

#define NSIG _NSIG

extern const char *const sys_siglist[NSIG]; //niby stare, ale ash chce
const static int sys_nsig = NSIG; //co te¿ ten ash wymy¶la..

typedef int sig_atomic_t;
typedef long int sigset_t;
typedef void (*sighandler_t)(int);

typedef long long int clock_t;

union sigval {
    int    sival_int;
    void  *sival_ptr;
};
typedef union sigval sigval_t;

struct siginfo {
    int      si_signo;
    int      si_errno;
    int      si_code;
    pid_t    si_pid;
    uid_t    si_uid;
    int      si_status;
    sigval_t si_value;
    void *   si_addr;
    int      si_band;
};

typedef struct siginfo siginfo_t;

struct sigaction {
    void   (*sa_handler)(int);
    void   (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t sa_mask;
    int      sa_flags;
};

int kill(pid_t pid, int sig);
int killpg(int pgrp, int sig);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigemptyset(sigset_t *set);
sighandler_t signal(int signum, sighandler_t handler);
int sigaction(int, const struct sigaction *,
                         struct sigaction *);



#define SIG_SETMASK 0

#endif

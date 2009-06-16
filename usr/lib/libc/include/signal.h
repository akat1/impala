#ifndef __SIGNAL_H
#define __SIGNAL_H

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h> //tak na razie...

#define NSIG _NSIG

extern const char *const sys_siglist[NSIG]; //niby stare, ale ash chce
const static int sys_nsig = NSIG; //co te� ten ash wymy�la..

typedef int sig_atomic_t;

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

int kill(pid_t pid, int sig);
int killpg(int pgrp, int sig);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int sigemptyset(sigset_t *set);
sighandler_t signal(int signum, sighandler_t handler);
int sigaction(int, const struct sigaction *,
                         struct sigaction *);


void sigreturn(void);
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);
int sigpending(sigset_t *set);
int siginterrupt(int sig, int flag);
int raise(int sig);
#endif

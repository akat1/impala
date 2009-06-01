#ifndef __SIGNAL_H
#define __SIGNAL_H

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

//To siginfo pochodzi z mana Linuksa... pewno wypada³o by mieæ w³asnes
struct siginfo {
    int      si_signo;  /* Numer sygna3u */
    int      si_errno;  /* Warto9|ae errno */
    int      si_code;   /* Kod sygna3u */
    pid_t    si_pid;    /* Id procesu wysy3aj+-cego */
    uid_t    si_uid;    /* Rzeczywisty ID ucytkownika wysy3aj+-cego procesu */
    int      si_status; /* Kod zakonczenia lub sygna3 */
    clock_t  si_utime;  /* Czas spedzony w przestrzeni ucytkownika */
    clock_t  si_stime;  /* Czas spedzony w przestrzeni systemu */
    sigval_t si_value;  /* Warto9|ae sygna3u */
    int      si_int;    /* sygna3 POSIX.1b */
    void *   si_ptr;    /* sygna3 POSIX.1b */
    void *   si_addr;   /* Adres pamieci, ktory spowodowa3 b3+-d */
    int      si_band;   /* Zdarzenie grupy (band event) */
    int      si_fd;     /* Deskryptor pliku */
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
int sigaction(int, const struct sigaction *restrict,
                         struct sigaction *restrict);



#define SIG_DFL 0
#define SIG_SETMASK 0

#endif

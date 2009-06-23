#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/list.h>
#include <string.h>

//__asm__( ".weak __pthread_rt");

int syscall(int SC, ...);
int main(int argc, char **argv, char **envp);
int _pthread_rt(void);

void _start(void);

int errno = 0;
static int retval=0;

FILE *_stdF[3]={NULL, NULL, NULL};
// = {{.fd=0, .status=_FST_OPEN|_FST_LINEBUF|_FST_TTY,
//                      .buf_size = BUFSIZ},
//                   {.fd=1, .status=_FST_OPEN|_FST_LINEBUF|_FST_TTY,
//                      .buf_size = BUFSIZ},
//                   {.fd=2, .status=_FST_OPEN|_FST_NOBUF|_FST_TTY,
//                      .buf_size = BUFSIZ}};

sighandler_t __sig_handlers[NSIG+1];

list_t __open_files;

int
_pthread_rt()
{
    return 0;
}

char **environ = NULL;
//char *tab2[2] = {NULL, NULL};
//char **environ = tab2;
#define MAX_ENV 256

int
syscall(int SC, ...)
{
    __asm__ (
        "movl %%ebx, %%eax;"
        " int $0x80"
        : "=a"(retval), "=c"(errno) 
        : "b"(SC)
    );
    return retval;
}

void
_start()
{
    char **argv;
    char **envp;
    __asm__(
        "movl %%edi, %0;"
        "movl %%esi, %1;"
        : "=m"(argv), "=m"(envp)
        :
    );
    int c = 0;
    for (int i = 0; argv[i]; i++) c++;
    //_pthread_rt();
    LIST_CREATE(&__open_files, FILE, L_open_files, FALSE);
    _stdF[0] = fdopen(0, "r");
    _stdF[1] = fdopen(1, "w");
    _stdF[2] = fdopen(2, "w");
///w teorii dla 0 i 1 powinno byæ dobrze ustawione, ale lepiej siê upewniæ:
    _stdF[0]->status =
    _stdF[1]->status = _FST_OPEN|_FST_LINEBUF|_FST_TTY;
    _stdF[2]->status = _FST_OPEN|_FST_NOBUF|_FST_TTY;
    //póki co:
    //environ = envp;
    environ = malloc(MAX_ENV*sizeof(char*));
    environ[0] = NULL;
    if(envp) {
        char **ee = envp;
        int i=0;
        for( ; *ee; i++, ee++)
            environ[i] = strdup(*ee);
        environ[i] = NULL;
    }
    char ex = main(c, argv, environ);
    syscall(SYS_exit, ex);
    for (;;); // tymczasowo
}


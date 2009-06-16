#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

//__asm__( ".weak __pthread_rt");

int syscall(int SC, ...);
int main(int argc, char **argv);
int _pthread_rt(void);

void _start(void);

int errno = 0;
static int retval=0;

int
_pthread_rt()
{
    return 0;
}

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

char *tab[2] = {NULL, NULL};

void
_start()
{
    _pthread_rt();
    //póki co:
//     environ = tab;
//     open("/dev/ttyv1", O_RDONLY /* | O_NOCTTY*/);   //stdin
//     open("/dev/ttyv1", O_WRONLY);   //stdout
//     open("/dev/ttyv1", O_WRONLY);   //stderr
//     printf("Environ: %x\n", 0);//environ);
//     close(2); close(1); close(0);
    char ex = main(0, tab);
    syscall(SYS_exit, ex);
    for (;;); // tymczasowo
}


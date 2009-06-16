#include <sys/types.h>
#include <sys/syscall.h>

__asm__( ".weak __pthread_rt");

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


void
_start()
{
    _pthread_rt();
    char ex = main(0, NULL);
    syscall(SYS_exit, ex);
    for (;;); // tymczasowo
}


#include <sys/types.h>
#include <sys/syscall.h>

int syscall(int SC, ...);
int main(int argc, char **argv);
void _start(void);

int errno = 0;
static int retval=0;

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
    char ex = main(0, NULL);
    syscall(SYS_exit, ex);
    for (;;); // tymczasowo
}


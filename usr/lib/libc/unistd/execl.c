#include <sys/syscall.h>
#include <unistd.h>
#include <stdarg.h>

#include "libc_syscall.h"

int
execl(const char *path, const char *arg, ...)
{
    const int MAX_ARGS = 128;
    char *argv[MAX_ARGS];
    va_list va;
    va_start(va, arg);
    for(int i=0; i<MAX_ARGS; i++) {
        argv[i] = va_arg(va, char*);
        if(!argv[i])
            break;
    }
    va_end(va);
    return syscall(SYS_execve, path, argv, environ); //environ przekazujemy?
}



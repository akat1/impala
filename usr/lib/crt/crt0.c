#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/list.h>
#include <string.h>
#include <stdio_private.h>
#include <stdlib_private.h>
#include <stdarg.h>

intptr_t syscall(int SC, ...);
int main(int argc, char **argv, char **envp);
void _start(void);
int errno = 0;
static intptr_t retval=0;
FILE *_stdF[3]={NULL, NULL, NULL};
list_t __open_files;
sighandler_t __sig_handlers[NSIG+1];
list_t __open_files;
list_t __mem_chunks;

char **environ = NULL;
#define MAX_ENV 256

intptr_t
syscall(int SC, ...)
{
    uintptr_t args[6] = {0, 0, 0, 0, 0, 0};
    va_list ap;
    va_start(ap, SC);
    for (int i = 0; i < 6; i++)
        args[i] = va_arg(ap, uintptr_t);
    va_end(ap);

    register uintptr_t reg_r10 __asm__("r10") = args[3];
    register uintptr_t reg_r8 __asm__("r8") = args[4];
    register uintptr_t reg_r9 __asm__("r9") = args[5];

    __asm__ volatile (
        "int $0x80"
        : "=a"(retval), "=c"(errno)
        : "a"((uintptr_t)SC),
          "D"(args[0]), "S"(args[1]), "d"(args[2]),
          "r"(reg_r10), "r"(reg_r8), "r"(reg_r9)
        : "r11", "memory"
    );
    return retval;
}

void
_start()
{
    char **argv;
    char **envp;
    __asm__ volatile(
        "movq %%rdi, %0;"
        "movq %%rsi, %1;"
        : "=m"(argv), "=m"(envp)
        :
    );
    int c = 0;
    for (int i = 0; argv[i]; i++) c++;
    LIST_CREATE(&__open_files, FILE, L_open_files, FALSE);
    LIST_CREATE(&__mem_chunks, mem_chunk_t, L_mem_chunks, FALSE);
    _stdF[0] = fdopen(0, "r");
    _stdF[1] = fdopen(1, "w");
    _stdF[2] = fdopen(2, "w");
/// w teorii dla 0 i 1 powinno być dobrze ustawione, ale lepiej się upewnić:
    _stdF[0]->status =
    _stdF[1]->status = _FST_OPEN|_FST_LINEBUF|_FST_TTY;
    _stdF[2]->status = _FST_OPEN|_FST_NOBUF|_FST_TTY;
    //póki co:
    environ = malloc(MAX_ENV*sizeof(char*));
    environ[0] = NULL;
    if(envp) {
        char **ee = envp;
        int i=0;
        for( ; *ee; i++, ee++)
            environ[i] = strdup(*ee);
        environ[i] = NULL;
    }
    exit(main(c, argv, environ));
    for (;;); 
}

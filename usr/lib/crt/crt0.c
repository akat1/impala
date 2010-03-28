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

int syscall(int SC, ...);
int main(int argc, char **argv, char **envp);
void _start(void);
int errno = 0;
static int retval=0;
FILE *_stdF[3]={NULL, NULL, NULL};
list_t __open_files;
sighandler_t __sig_handlers[NSIG+1];
list_t __open_files;
list_t __mem_chunks;

char **environ = NULL;
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


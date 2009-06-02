#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>

#define print(fd, str) write(fd, str, strlen(str))

const char *msg = "fork () test\n";
void
tmain()
{
    print(0, "thread\n");
    while(1);
}


static int retval=0;

char hex[] = "0123456789abcdefGH";

void
print32(char *str, uint32_t val)
{
    str[0] = '0';
    str[1] = 'x';
    int i = 9;
    str[i--] = hex[val % 0x10]; val /= 0x10;
    str[i--] = hex[val % 0x10]; val /= 0x10;
    str[i--] = hex[val % 0x10]; val /= 0x10;
    str[i--] = hex[val % 0x10]; val /= 0x10;
    str[i--] = hex[val % 0x10]; val /= 0x10;
    str[i--] = hex[val % 0x10]; val /= 0x10;
    str[i--] = hex[val % 0x10]; val /= 0x10;
    str[i--] = hex[val % 0x10]; val /= 0x10;
}

void
printstack(uint32_t *v, int n)
{
    static char string[128];
    char *str = string;
    memset(string, 0, 128);
    for (int i = 0; i < n; i++) {
        print32(str, v[i]);
        str += 10;
        *str = ' ';
        str++;
    }
    *str = '\n';
    print(0, string);
}


uint32_t
getesp()
{
    uint32_t r;
    __asm__ ( "movl %%esp, %0"
                : "=rm"(r));
    return r;
}


int
_syscall(int SC, ...)
{
    enum { N = 6 };
    uint32_t x;
#ifdef __Impala__
    __asm__ (
        "movl %%ebx, %%eax;"
        " int $0x80"
        : "=a"(retval), "=c"(errno)
        : "b"(SC)
    );
#else
    retval = fork();
#endif
    __asm__( "movl %%ebp, %0" : "=rm"(x));

    if (retval == 0) {
        print(0, "child ret\n");
        printstack(getesp(), N);
        printstack(&x, 1);
    } else {
        print(0, "fath ret\n");
        printstack(getesp(), N);
        printstack(&x, 1);
    }
    return retval;
}

pid_t
_fork()
{
    return _syscall(SYS_fork, tmain);    
}

int
main(int argc, char **v)
{
    int fd = open("/dev/ttyv0", 0, 0);
    print(fd, msg);
    pid_t p = fork();
    if (p == 0) {
        print(0, "child\n");
        execve("/bin/test", NULL, NULL);
    } else
    if (p == -1) {
        print(0, "error\n");
    } else {
        print(0, "father\n");
    }
    while(1);
    return 0;
}

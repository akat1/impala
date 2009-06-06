#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>

#define print(fd, str) write(fd, str, strlen(str))

char hex[] = "0123456789abcdefGH";

void
print32(char *str, uint32_t val)
{
    str[0] = '0';
    str[1] = 'x';
    int i = 10;
    str[i--] = 0;
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
printV(const char *msg, int v)
{
    char val[10];
    print(0, msg);
    print32(val, v);
    print(0, val);
    print(0, "\n");
}

#define _E(ret, msg) if (ret == -1) {\
    print(0, msg);\
    return -1;\
    }


void
tmain()
{
    int fd = open("/dev/tty", O_RDWR);
    int err = errno;
    printV("fd w w±tku: ", fd);
    printV("errno: ", err);
    print(fd,"elo320\n\n");
    print(fd,"\tSiemanizator ;D... 1234567890\n\n");
    while(1);
}

int
main(int argc, char **v)
{
    int fd = 0;
#ifdef __Impala__
    fd = open("/dev/ttyv1", O_RDWR /* | O_NOCTTY*/, 0);
#endif
    tid_t t = thr_create(tmain, 0, 0, 0);
    printV("Utw. w±tek:", t);
    char buf[32];
    while(1) {
        int s = read(0, buf, 32);
        write(0, buf, s);
    }
    
    return 0;
}

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
    print(0,"elo");
    while(1);
}

int
main(int argc, char **v)
{
    int fd = 0;
#ifdef __Impala__
    fd = open("/dev/ttyv0", 0, 0);
#endif
    tid_t t = thr_create(tmain, 0, 0, 0);
    return 0;
}

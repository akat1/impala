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
    print(1, msg);
    print32(val, v);
    print(1, val);
    print(1, "\n");
}

#define _E(ret, msg) if (ret == -1) {\
    print(1, msg);\
    return -1;\
    }

int
main(int argc, char **v)
{
    char msg[100];
    int fd = 0;
#ifdef __Impala__
    open("/dev/ttyv1", O_RDWR);
    open("/dev/ttyv1", O_RDWR);
#endif
    key_t k = ftok("/sbin/init", 1);

    int msg_id = msgget(k, IPC_CREAT|S_IRWXU|S_IRWXG|S_IRWXO);
    _E(msg_id, "msgget error\n");
    _E(msgrcv(msg_id, msg, sizeof(msg), 0,  0), "msgsnd error\n");
    printV("key: ", k);
    printV("msg id: ", msg_id);
    print(1, "msg: ");
    print(1, msg);
    print(1, "\n");
#ifdef __Impala__
    while(1);
#endif
    return 0;
}

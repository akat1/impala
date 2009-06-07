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


void
tmain()
{
    int fd = open("/dev/tty", O_RDWR);
    int err = errno;
    printV("fd w w±tku: ", fd);
    printV("errno: ", err);
    print(fd,"elo320\n\n");
    print(fd,"\tSiemanizator ;D... 1234567890\n\n");
    printf("Test oto jest %i\n", 12);
    while(1);
}

//inputline z vttest
void
inputline(char *s)
{
  do {
    int ch;
    char *d = s;
    while ((ch = getchar()) != EOF && ch != '\n') {
      if ((d - s) < BUFSIZ - 2)
        *d++ = (char) ch;
    }
    *d = 0;
  } while (!*s);
}


int
main(int argc, char **v)
{
#ifdef __Impala__
    open("/dev/ttyv1", O_RDONLY /* | O_NOCTTY*/);   //stdin
    open("/dev/ttyv1", O_WRONLY);   //stdout
    open("/dev/ttyv1", O_WRONLY);   //stderr
#endif
    tid_t t = thr_create(tmain, 0, 0, 0);
    printV("Utw. w±tek:", t);

    char buf[512];
    char *const argv[]={NULL};
    char *const env[]={NULL};
    while(1) {
        inputline(buf);
        int s = strlen(buf);
        buf[s]=0;
        if(!strcmp(buf, "quit"))
            exit(0);
        if(!strcmp(buf, "test")) {
            if(!fork()) {
                execve("/bin/vttest", argv, env);
                printf("Nie uda³o siê uruchomiæ programu...");
                exit(-1);
            } else {
                printf("Uruchamiam ma³e co nieco...\n");
                while(1);
            }
        }
        if(!strcmp(buf, "test2")) {
            if(!fork()) {
                execve("/bin/test", argv, env);
                printf("Nie uda³o siê uruchomiæ programu...");
                exit(-1);
            } else {
                printf("Uruchamiam ma³e co nieco...\n");
            }
        }
        printf("%s", buf);
    }
    
    return 0;
}

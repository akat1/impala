#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

const char data[] = "Hello, World\n";
const char data2[] = "OK\n";
static char sbuf[32];

char *itoa(int num);
#define TRUE 1
#define FALSE 0

char
*itoa(int num)
{
    char *c=sbuf+31;
    bool min = FALSE;
    *c = 0;
    c[-1] = '0';
    if(num == 0)
        return c-1;
    if(num < 0) {
        min = TRUE;
        num = -num;
    }
    while(num>0) {
        *(--c) = '0' + num%10;
        num/=10;
    }
    if(min)
        *(--c) = '-';
    return c;
}

extern int errno;

int
main(int argc, char **argv)
{
    int fd = open("/dev/ttyv0", 0, 0);
    int fd2 = open("/etc/passwd", 0, 0);
    char buf[128];
    read(fd2, buf, 127);
    char *b = itoa(fd);
    write(0, b, strlen(b));
    b = itoa(fd2);
    write(0, b, strlen(b));
    write(0, buf, strlen(buf));
    write(0, data2, strlen(data2));
    write(0, data, strlen(data));
    while(1) {int l = read(fd, buf, 127);
    write(fd, buf, l); }
    while(1);
    return 0;
}

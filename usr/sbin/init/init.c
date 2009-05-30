#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define print(fd, str) write(fd, str, strlen(str))

#if 0
const char *msg = "fork () test\n";
int
main(int argc, char **v)
{
    int fd = open("/dev/ttyv0", 0, 0);
    print(fd, msg);
    pid_t p = fork();
    if (p == 0) {
        print(fd, "I am father\n");
    } else
    if (p == -1) {
        print(fd, "error\n");
    } else {
        print(fd, "child\n");
    }
    while(1);
    return 0;
}


#else
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

void recur(int times);

void
recur(int times)
{
    if(times == 0)
        return;
    int tab[1024];
    for(int i=1; i<10; i++)
        tab[i]+=tab[i+1]+tab[i-1];
    recur(times-1);
}

int
main(int argc, char **argv)
{
    int fd = open("/dev/ttyv0", O_RDWR, 0);
    int fd2 = open("/etc/passwd", O_RDONLY, 0);
    char buf[128];
    read(fd2, buf, 127);
    char *b = itoa(fd);
    write(0, b, strlen(b));
    b = itoa(fd2);
    write(0, b, strlen(b));
    write(0, buf, strlen(buf));
    write(0, data2, strlen(data2));
    write(0, data, strlen(data));
    close(open("pliczek", O_CREAT, 0744));
    int fd3 = open("/", 0, 0);
    dirent_t dents[20];
    int w = getdents(fd3, dents, 20*sizeof(dirent_t));
    for(int i=0; i<w/sizeof(dirent_t); i++) {
        dirent_t *d = &dents[i];
        write(0, d->d_name, strlen(d->d_name));
        write(0, "\n", 1);
    }
    write(0, "A oto dev:\n", strlen("A oto dev:\n"));
    fd3 = open("/dev/", 0, 0);
    w = getdents(fd3, dents, 20*sizeof(dirent_t));
    for(int i=0; i<w/sizeof(dirent_t); i++) {
        dirent_t *d = &dents[i];
        write(0, d->d_name, strlen(d->d_name));
        write(0, "\n", 1);
    }
//    recur(10);
    while(1) {int l = read(fd, buf, 127);
    write(fd, buf, l); }
    while(1);
    return 0;
}
#endif

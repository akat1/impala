#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <sys/thread.h>

void tmain(void);
void inputline(char *s);

int
main(int argc, char **v)
{
    char *args[] = {
        "/bin/sh",
        NULL
    };
    char *env[] = {
        "TERM=vt100-8025",
        NULL
    };
    if (argc != 2) {
        printf("%s terminal-device\n", v[0]);
        return -1;
    }
    printf("%s: preparing to run /sbin/cat on %s\n", v[0], v[1]);
    close(0);
    close(1);
    close(2);
    setsid();
    open(v[1], O_RDONLY);
    open(v[1], O_WRONLY); 
    open(v[1], O_WRONLY); 
    execve("/bin/cat", args , env);
    return 0;
}

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
    if (argc < 3) {
        return -1;
    }
    close(0);
    close(1);
    close(2);
    // je¿eli jeste¶my leaderem grupy, to nie mo¿emy zrobiæ setsid -> trik ;)
    pid_t p = fork();
    if (p) exit(0); 
    setsid();
    open(v[1], O_RDONLY);
    open(v[1], O_WRONLY);
    open(v[1], O_WRONLY);
    v++;
    execve(v[2], v , environ);
    return 0;
}

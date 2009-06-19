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

void
tmain()
{
    printf("elo320\n\n");
    printf("\tSiemanizator ;D... 1234567890\n\n");
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

void handler(int sig)
{
    printf("HELLO WORLD!\n");
}

int
main(int argc, char **v)
{
#ifdef __Impala__
    setsid();
    open("/dev/ttyv1", O_RDONLY /* | O_NOCTTY*/);   //stdin
    open("/dev/ttyv1", O_WRONLY);   //stdout
    open("/dev/ttyv1", O_WRONLY);   //stderr
#endif
    thr_create(tmain, 0, 0, 0);

    int p;
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
        if(!strcmp(buf, "sh")) {
            if(!fork()) {
                execve("/bin/sh", argv, env);
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
        if(!strcmp(buf, "signal")) {
            printf("SIGHUP:\n");
            signal(SIGHUP, handler);
            kill(getpid(), SIGHUP);
            if ( ! (p=fork()) )
            {
                while(1);
            }
            kill(p, SIGSTOP);
            printf("STOPPED!\n");
            kill(p, SIGCONT);
            printf("CONT!\n");
            kill(p, SIGKILL);
            printf("Bye: %i\n", p);
        }
        printf("%s", buf);
    }
    
    return 0;
}

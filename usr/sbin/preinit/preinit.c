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

#define dist_tar "/mnt/fd0/impala/dist.tar.gz"
#define tar "/mnt/fd0/impala/tar"
#define init "/sbin/init"
#define gzip "/mnt/fd0/impala/gzip"

int
main(int argc, char **v)
{
    char * const argv[] = {
        "example",
        NULL,
        "xVf",
        dist_tar,
        NULL
    };
    char * const envp[] = {
        "GZIP=" gzip,
        NULL
    };
    pid_t p;
    int status;
#ifdef __Impala__
    setsid();
    open("/dev/ttyv1", O_RDONLY|O_NOCTTY);  //stdin
    open("/dev/ttyv1", O_WRONLY|O_NOCTTY);  //stdout
    open("/dev/ttyv1", O_WRONLY|O_NOCTTY);  //stderr
#endif
    if (getpid() != 1) {
        printf("sorry, only kernel can run this program\n");
        return -1;
    }
    printf("Impala floppy preinit\n");
    printf(" distribution archive: %s\n", dist_tar);
    printf(" tape archivizer: %s\n", tar);
    printf(" init program: %s\n", init);
    printf(" gzip program: %s\n", gzip);

    printf(" * extracting distribution archive\n");
    if ( (p = fork()) == 0 ) {
        chdir("/");
        execve(gzip, argv, envp);
        printf("preinit: cannot run tar\n");
        exit(-1);
    } else
    if (p == -1) {
        printf("preinit: cannot fork\n");
        while (1);
    }
    waitpid(p, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status)!=0 ) {
        printf("preinit: tar exited with non zero code\n");
        while(1);
    }
    printf(" * executing %s\n", init);
    close(0);
    close(1);
    close(2);
    execve(init, NULL, NULL);
    return 0;
}

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

const char *dist_tar = "/mnt/fd0/impala/dist.tar";
const char *tar = "/mnt/fd0/impala/tar";
const char *init = "/sbin/init";

int
main(int argc, char **v)
{
    char **argv[] = {
        "tar",
        "xvf",
        dist_tar,
        NULL
    };
    pid_t p;
#ifdef __Impala__
    setsid();
    open("/dev/ttyv1", O_RDONLY|O_NOCTTY);  //stdin
    open("/dev/ttyv1", O_WRONLY|O_NOCTTY);  //stdout
    open("/dev/ttyv1", O_WRONLY|O_NOCTTY);  //stderr
#endif
    printf("Impala floppy preinit\n");
    printf(" distribution archive: %s\n", dist_tar);
    printf(" tape archivizer: %s\n", tar);
    printf(" init program: %s\n", init);

    printf(" * extracting distribution archive\n");
    if ( (p = fork()) == 0 ) {
        chdir("/");
        execve(tar, argv, NULL);
        printf("cannot run tar\n");
        exit(-1);
    } else
    if (p == -1) {
        printf("cannot fork\n");
        while (1);
    }
    sleep(3);
    printf(" * executing %s\n", init);
    close(0);
    close(1);
    close(2);
    execve(init, NULL, NULL);
    return 0;
}

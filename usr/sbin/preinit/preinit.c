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

#define syspack "/mnt/fd0/impala/syspack.tar.gz"
#define tar "/mnt/fd0/impala/tar"
#define init "/sbin/init"
#define gzip "/mnt/fd0/impala/minigzip"

int
main(int argc, char **v)
{
    char *argv[] = {
        "tar",
        "xVf",
        syspack,
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
    printf(" * extracting system from %s\n", syspack);
    if ( strlen(syspack) > 2) {
        int l = strlen(syspack);
        if (syspack[l-2] == 'g' && (syspack[l-1]) == 'z') {
            argv[1] = "zxVf";
        }
    }
#if 0
    printf("\033[2Kinvoking gzip\r");
    fflush(stdout);
    p = fork();
    if ( p == 0 ) {
        char *gzipv[] = {
            "gzip",
            "-d",
            NULL
        };
        chdir("/");
        close(0);
        close(1);
        open(syspack, O_RDONLY);
        open("/dist.tar", O_WRONLY|O_CREAT, 0755);
        execve(gzip, gzipv, NULL);
        fprintf(stderr, " * system cannot run gzip\n");
    }
    waitpid(p, &status, 0);
#endif
    printf("\033[2Kinvoking tar %s %s\r", argv[1], argv[2]);
    fflush(stdout);
    
    if ( (p = fork()) == 0 ) {
        chdir("/");
        execve(tar, argv, envp);
        printf(" * system cannot startup (cannot run tar)\n");
        perror("execve");
        exit(-1);
    } else
    if (p == -1) {
        printf(" * system cannot startup (cannot fork)\n");
        perror("fork");
        while (1);
    }
    waitpid(p, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status)!=0 ) {
        printf(" * tar exited with non zero status\n");
        while(1);
    }
    printf(" * executing %s\n", init);
    close(0);
    close(1);
    close(2);
    execve(init, NULL, NULL);
    while(1);
    return 0;
}

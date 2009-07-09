#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int, char **);

int
main(int argc, char **argv)
{
    key_t ipckey = ftok("/etc/rc.ttyv", 15);
    int msg;
    if (argc == 2 && strcmp(argv[1],"create")==0) {
        msg = msgget(ipckey, IPC_CREAT|IPC_EXCL);
        if (msg == -1 && errno == EEXIST) {
            printf("message queue already exists\n");
            return -1;
        } else
        if (msg == -1) {
            perror("msgget");
            return -1;
        }
    } else
    if (argc < 2) {
        printf("%s create\n", argv[0]);
        printf("%s send MSG\n", argv[0]);
        printf("%s recv\n", argv[0]);
    }
    return 0;
}

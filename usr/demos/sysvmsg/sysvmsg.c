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
    struct message {
        unsigned long first;
        char string[128];
    };
    key_t ipckey = ftok("/etc/rc.ttyv", 15);
    int msgid;
    if (argc == 2 && strcmp(argv[1],"create")==0) {
        msgid = msgget(ipckey, IPC_CREAT|IPC_EXCL|0777);
        if (msgid == -1 && errno == EEXIST) {
            printf("message queue already exists\n");
            return -1;
        } else
        if (msgid == -1) {
            perror("msgget");
            return -1;
        }
    } else
    if (argc == 3 && strcmp(argv[1],"send")==0) {
        struct message MSG;

        msgid = msgget(ipckey, 0);
        if (msgid == -1 && errno == ENOENT) {
            printf("message queue does not exists\n");
            return -1;
        } else
        if (msgid == -1) {
            perror("msgget");
            return -1;
        }
        MSG.first = argv[2][0];
        strncpy(MSG.string, argv[2], sizeof(MSG.string)-1);
        if (msgsnd(msgid, &MSG, sizeof(MSG), 0) == -1 ) {
            perror("msgsnd");
            return -1;
        }
    } else
    if (argc == 2 && strcmp(argv[1],"recv")==0) {
        struct message MSG;

        msgid = msgget(ipckey, 0);
        if (msgid == -1 && errno == ENOENT) {
            printf("message queue does not exists\n");
            return -1;
        } else
        if (msgid == -1) {
            perror("msgget");
            return -1;
        }
        if (msgrcv(msgid, &MSG, sizeof(MSG), 0, 0) == -1) {
            perror("msgrcv");
            return -1;
        }
        printf("received [%s]\n", MSG.string);
    } else
    if (argc < 2) {
        printf("%s create\n", argv[0]);
        printf("%s send MSG\n", argv[0]);
        printf("%s recv\n", argv[0]);
    }
    return 0;
}

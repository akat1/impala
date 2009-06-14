#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


void
printV(const char *msg, int v)
{
    printf("%s: %x\n", msg, v);
}

#define _E(ret, msg) if (ret == -1) {\
    printf("%s", msg);\
    return -1;\
    }


int
main(int argc, char **v)
{
    char msg[100];
    int fildes[2];
    const int BSIZE = 100;
    char buf[BSIZE];
    ssize_t nbytes;
    int status;

    status = pipe(fildes);
    if (status == -1 ) {
        /* an error occurred */
        printf("Error...\n");
    }
    printf("pipe: %i %i\n", fildes[0], fildes[1]);
    switch (fork()) {
    case -1: /* Handle error */
        printf("Fork error...\n");
        break;

///@todo  SHM: uprzejmie proszê o fd_clone, aby nie musieæ hackowaæ close...
        

    case 0:  /* Child - reads from pipe */
//        close(fildes[1]);                       /* Write end is unused */
        nbytes = read(fildes[0], buf, BSIZE);   /* Get data from pipe */
        /* At this point, a further read would see end of file ... */
//        close(fildes[0]);                       /* Finished with pipe */
        printf("Child: read res - %s, %i\n", buf, nbytes);
        exit(EXIT_SUCCESS);


    default:  /* Parent - writes to pipe */
//        close(fildes[0]);                       /* Read end is unused */
        nbytes = write(fildes[1], "Hello world\n", 12);  /* Write data on pipe */
//        close(fildes[1]);                       /* Child will see EOF */
        printf("Father: after write, res = %i\n", nbytes);
        exit(EXIT_SUCCESS);
    }    
    
    key_t k = ftok("/sbin/init", 1);
    int msg_id = msgget(k, IPC_CREAT|S_IRWXU|S_IRWXG|S_IRWXO);
    _E(msg_id, "msgget error\n");
    _E(msgrcv(msg_id, msg, sizeof(msg), 0,  IPC_NOWAIT), "msgsnd error\n");
    printV("key: ", k);
    printV("msg id: ", msg_id);
    printf("msg: %s\n", msg);
    return 0;
}

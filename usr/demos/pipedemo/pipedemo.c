#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>



int
main(int argc, char **v)
{
    int fildes[2];
    const int BSIZE = 100;
    char buf[BSIZE];
    ssize_t nbytes;
    int status;

    status = pipe(fildes);
    if (status == -1 ) {
        /* an error occurred */
        perror("pipe");
    }
    printf("pipe: %i %i\n", fildes[0], fildes[1]);
    switch (fork()) {

    case -1: /* Handle error */
        perror("fork");
        break;

    case 0:  /* Child - reads from pipe */
        close(fildes[1]);                       /* Write end is unused */
        nbytes = read(fildes[0], buf, BSIZE);   /* Get data from pipe */
        /* At this point, a further read would see end of file ... */
        close(fildes[0]);                       /* Finished with pipe */
        printf("Child: read res - %s, %i\n", buf, nbytes);
        exit(EXIT_SUCCESS);


    default:  /* Parent - writes to pipe */
        close(fildes[0]);                       /* Read end is unused */
        nbytes = write(fildes[1], "Hello world", 12);  /* Write data on pipe */
        close(fildes[1]);                       /* Child will see EOF */
        printf("Father: after write, res = %i\n", nbytes);
        exit(EXIT_SUCCESS);
    }    
    return -1;
}

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int
system(const char *command)
{
    switch(fork()) {
        case -1:
            return -1;
        case 0: {
            execl("/bin/sh", "sh", "-c", command, NULL);
            break;
        }
        default: {
            int status = 0;
            waitpid(-1, &status, 0);
            return WEXITSTATUS(status);
        }
    }
    return -1;
}

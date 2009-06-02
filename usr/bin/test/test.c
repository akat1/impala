#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>

#define print(fd, str) write(fd, str, strlen(str))

const char *msg = "bin/test: Poczekaj 5 sekund, zrobie rozwidlenie.\n"
                    "Potem jedna z moich osobowosci zrobi cos brzydkiego.\n";

int
main(int argc, char **v)
{
    int fd = open("/dev/ttyv0", 0, 0);
    print(fd, msg);
    int i = fork();
    sleep(5);
    print(fd, "and wait demo?\n");
    if (i == 0) {
        char *d = 0xfffffff0;
        *d = 5;
    }
    return 0;
}

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

char *optarg;
int optind, opterr, optopt, optreset;

int
getopt(int argc, char * const argv[], const char *optstring)
{
    return -1;
}

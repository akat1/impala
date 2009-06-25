#include <sys/utsname.h>
#include <stdio.h>


int
main(int argc, char **v)
{
    struct utsname u;
    uname(&u);
    // czekamy na getopt
    printf("%s %s %s %s %s\n", 
        u.sysname, u.nodename, u.release, u.version,
        u.machine);
    return 0;
}


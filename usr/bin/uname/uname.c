/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h>

static void
usage(const char *progname)
{
    fprintf(stderr, "usage: %s [-snrvma]\n", progname);
}

/* Basing on
 * http://pubs.opengroup.org/onlinepubs/007904975/utilities/uname.html */

int
main(int argc, char **argv)
{
    int ch, all, machine, node, release, implementation = 1, version;
    struct utsname u;

    while ((ch = getopt(argc, argv, "snrvma")) != -1) {
        switch(ch) {
        case 's':
            implementation = 1;
            break;
        case 'n':
            node = 1;
            break;
        case 'r':
            release = 1;
            break;
        case 'v':
            version = 1;
            break;
        case 'm':
            machine = 1;
            break;
        case 'a':
            all = 1;
            break;
        case '?':
            /*FALLTHROUGH*/
        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (uname(&u) != 0) {
        fprintf(stderr, "uname(2) call failed!\n");
        exit(EXIT_FAILURE);
    }

    if (all == 1) {
        printf("%s %s %s %s %s\n", u.sysname, u.nodename, u.release,
          u.version, u.machine);
    } else {
        if (node == 1)
            printf("%s ", u.nodename);
        if (release == 1)
            printf("%s ", u.release);
        if (version == 1)
            printf("%s ", u.version);
        if (machine == 1)
            printf("%s ", u.machine);
        if (implementation == 1)
            printf("%s ", u.sysname);
        printf("\n");
    }

    return EXIT_SUCCESS;
}

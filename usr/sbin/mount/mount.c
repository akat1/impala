/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/mount.h>

/*
 * XXX:
 *
 * add flags
 */

void print_mountinfo(void);

void
print_mountinfo(void)
{
    enum {  
        N = 20
    };
    int i;
    int n;
    int off;
    struct mountinfo minfo[N];
    printf("%8s %8s %8s %s\n", 
        "DEV", "TYPE", "OPTIONS", "POINT"
    );
    for( off = 0, n = 0; (n = getmountinfo(off, minfo,N)) && n > 0; off += n ) {
        for (i = 0; i < n; i++) {
            printf("%8s %8s %8s %s\n",
                minfo[i].dev, minfo[i].type, "", minfo[i].mpoint);
        }
    }
}

int
main(int argc, char **argv)
{
    int ch;
    int error;
    char *type = "";
    char *mountpoint = "";
    /* what a stupid name :-) */
    char *mounted = NULL;

    if (argc == 1) {
        print_mountinfo();
        return 0;
    }
        

    while((ch = getopt(argc, argv, "t:")) != -1) {
        switch(ch) {
        case 't':
            type = strdup(optarg);
            if (type == NULL) {
                fprintf(stderr, "no memory");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            fprintf(stderr, "unknown switch", ch);
            exit(EXIT_FAILURE);
        }
    }

    argc -= optind;
    argv += optind;

    switch(argc) {
    case 1:
        mountpoint = argv[0];
        break;
    case 2:
        mountpoint = argv[1];
        mounted = argv[0];
        break;
    default:
        fprintf(stderr, "print help message someday (tm)\n");
        error = EXIT_FAILURE;
        goto out;
    }

    error = mount(type, mountpoint, 0, mounted);

out:
    free(type);
    return error;
}

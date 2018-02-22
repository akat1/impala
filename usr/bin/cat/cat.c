/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

/* XXX: missing -u option
 * http://pubs.opengroup.org/onlinepubs/007904975/utilities/cat.html
 */

#include <stdio.h>
#include <string.h>

const char *progname;

static int
readfile(const char *filename)
{
    char buf[1024];
    int n;
    FILE *f;

    /* read from STDIN */
    if (filename == NULL || strcmp(filename, "-") == 0)
        f = stdin;
    else
        f = fopen(filename, "r");

    if (f == NULL) {
        fprintf(stderr, "%s: cannot open file %s\n", progname, filename);
        return 1;
    }

    while ( (n = fread(buf, 1, 1024-1, f)) > 0 ) {
        buf[n] = 0;
        puts(buf);
    }

    if (f != stdin)
        fclose(f);

    return 0;
}

int
main(int argc, char **argv)
{
    int i, r = 0;
    progname = argv[0];

    if (argc < 2)
        readfile(NULL);
    else {
        for (i = 1; i < argc; i++) {
            if (readfile(argv[i]) != 0)
                r = 1;
        }
    }

    return r;
}

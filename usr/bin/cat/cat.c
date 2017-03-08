#include <stdio.h>

int
main(int argc, char **v)
{
    FILE *f;
    if (argc != 2) {
        f = stdin;
    } else {
        f = fopen(v[1], "r");
    }
    if (f == NULL) {
        printf("%s: cannot open file %s\n", v[0], v[1]);
        return -1;
    }
    char buf[1024];
    int n;
    while ( (n = fread(buf, 1, 1024-1, f)) > 0 ) {
        buf[n] = 0;
        printf("%s", buf);
    }
    fclose(f);
    return 0;
}

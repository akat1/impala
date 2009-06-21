#include <stdio.h>

int
main(int argc, char **v)
{
    FILE *f;
    char ch;
    if (argc != 2) {
        f = stdin;
    } else {
        f = fopen(v[1], "r");
    }
    if (f == NULL) {
        printf("%s: cannot open file %s\n", v[0], v[1]);
        return -1;
    }
    int c;
    while ( (c = fgetc(f)) != -1 ) {
        printf("%c", c);
    }
    return 0;
}

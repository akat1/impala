#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <ctype.h>
#define iscool(c) (c != '\\' && c != '\'' && isprint(c))

int
main(int argc, char **argv)
{
    enum {
        COL_MAX = 13,
    };
    int ch;
    int col = 0;
    if (argc != 2) return -1;
    FILE *source = fopen(argv[1], "r");
    if (source == NULL) return -1;
    printf("/* Generated from [%s] */\n", argv[1]);
    printf("unsigned char image[] = {\n    ");
    while ( !feof(source) && (ch = fgetc(source)) != -1 ) {
        if (iscool(ch)) 
            printf("'%c', ", ch);
            else printf("%3u, ", ch);
       if (col++ == COL_MAX) {
            col = 0;
            printf("\n    ");
       }
    }
    printf("\n};\n");
    printf("unsigned int image_size = sizeof(image);\n");
    fclose(source);
    return 0;
}

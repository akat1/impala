#include <sys/types.h>
#include <stdio.h>
#include <string.h>

//todo: lista b³êdów - ich opisów

void
perror(const char *s)
{
    if(!s)
        fprintf(stderr, "%s", "ERROR");
    else
        fprintf(stderr, "%s: %s", "ERROR", s);
}

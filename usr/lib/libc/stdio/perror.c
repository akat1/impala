#include <sys/types.h>
#include <stdio.h>
#include <string.h>

//todo: lista b��d�w - ich opis�w

void
perror(const char *s)
{
    if(!s)
        fprintf(stderr, "%s", "ERROR");
    else
        fprintf(stderr, "%s: %s", "ERROR", s);
}

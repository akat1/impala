#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//todo: lista b��d�w - ich opis�w

void
perror(const char *s)
{
    if(!s)
        fprintf(stderr, "%s\n", strerror(errno));
    else
        fprintf(stderr, "%s: %s\n",s, strerror(errno));
}

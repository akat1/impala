#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//todo: lista b³êdów - ich opisów

void
perror(const char *s)
{
    if(!s)
        fprintf(stderr, "%s\n", strerror(errno));
    else
        fprintf(stderr, "%s: %s\n",s, strerror(errno));
}

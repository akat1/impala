#include <stdlib.h>
#include <string.h>

char **environ;

char *
getenv(const char *name)
{
    char **ptr = environ;
    size_t nlen = strlen(name);
    while(*ptr) {
        if(!strncmp(name, *ptr, nlen) && (*ptr)[nlen]=='=')
            return &(*ptr)[nlen+1];
        ptr++;
    }
    return NULL;
}
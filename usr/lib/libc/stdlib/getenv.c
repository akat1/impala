#include <stdlib.h>
#include <string.h>

char *tab2[2] = {NULL, NULL};
char **environ = tab2;

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

#include <string.h>
#include <unistd.h>

char  *
strdup(const char *s)
{
    size_t len = strlen(s);
    char *res = malloc(len+1);
    if(!res)
        return NULL;
    strcpy(res, s);
    return res;
}
#include <string.h>

char *
strsep(char **stringp, const char *delim)
{
    if(!stringp)
        return NULL;
    char *res = *stringp;
    *stringp = strpbrk(*stringp, delim);
    if(*stringp)
        **stringp = '\0';
    return res;
}

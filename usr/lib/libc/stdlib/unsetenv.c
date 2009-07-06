#include <stdlib.h>
#include <string.h>
#include <errno.h>
extern char **environ;

static char **_last_env(char **e);

char **
_last_env(char **e) {
    char **last = e;
    while(*e) {
        last = e;
        e++;
    }
    return last;        
}

int
unsetenv(const char *name)
{
    if(strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }
    char **e = environ;
    size_t nlen = strlen(name);
    while(*e) {
        if(!strncmp(*e, name, nlen) && (*e)[nlen]=='=') {
            char **la = _last_env(e);
            *e = *la;
            *la = NULL;
            return 0;
        }
        e++;
    }
    return 0;
}

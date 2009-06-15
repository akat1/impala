#include <stdlib.h>
#include <string.h>
#include <errno.h>
extern char **environ;

static char *get_espace(size_t s);

char *
get_espace(size_t s)
{
    ///jak zmieniæ? co z alokacj± / zwalnianiem? póki co naiwnie:
    return malloc(s); ///nie powinno tak byæ..
}

int
setenv(const char *name, const char *value, int overwrite)
{
    if(strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }
    char **e = environ;
    size_t nlen = strlen(name);
    size_t dlen = nlen + strlen(value) + 2;
    while(*e) {
        if(!strncmp(*e, name, nlen) && (*e)[nlen]=='=') {
            if(!overwrite)
                return 0;

            char *new_entry = get_espace(dlen);
            sprintf(new_entry, "%s=%s", name, value);
            *e = new_entry; //co ze star± informacj±?
            return 0;
        }
    }
    //ok, pusto..
    char *new_entry = get_espace(dlen);
    sprintf(new_entry, "%s=%s", name, value);
    *e = new_entry; //co ze star± informacj±?
    *(++e) = NULL; // a co jak siê nie mie¶cimy ju¿... jakie du¿e jest environ?
    return 0;
}
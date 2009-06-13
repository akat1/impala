#include <unistd.h>
#include <string.h>

int
gethostname(char *name, size_t len)
{
    const char *hname = "<HOSTNAME>";
    if(len>strlen(hname)+1)
        len = strlen(hname)+1;
    strncpy(name, hname, len);
    return 0;
}

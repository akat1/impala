#include <unistd.h>
#include <sys/syscall.h>

char *
getcwd(char *buf, size_t size)
{
    if(!buf)
        return NULL;
    if(!syscall(SYS_getcwd, buf, size))
        return buf;
    return NULL;
}

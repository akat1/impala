#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int
closedir(DIR* dir)
{
    if(!dir) {
        errno = EINVAL;
        return -1;
    }
    return close(dir->fd);
}

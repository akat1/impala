#include <unistd.h>
#include <dirent.h>
#include <sys/syscall.h>

dirent_t _dir;

struct dirent *
readdir(DIR *dirp)
{
    int res = syscall(SYS_getdents, dirp->fd, &_dir, sizeof(struct dirent));
    if(res == 0)
        return NULL;
    return &_dir;
}

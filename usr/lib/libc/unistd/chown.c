#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

int
chown(const char *path, uid_t owner, gid_t group)
{
    return syscall(SYS_chown, path, owner, group);
}

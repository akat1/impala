#include <pwd.h>

struct passwd onlyuser = {
    .pw_name = "toor",
    .pw_uid = 0,
    .pw_gid = 0,
    .pw_dir = "/",
    .pw_shell = "/bin/ash"
};

struct passwd *
getpwnam(const char *name)
{
    return &onlyuser;   ///@todo impl. getpwnam
}

#include <pwd.h>

struct passwd onlyuser = {
    .pw_name = "toor",
    .pw_uid = 0,
    .pw_gid = 0,
    .pw_dir = "/",
    .pw_shell = "/bin/sh"
};

struct passwd *
getpwuid(uid_t uid)
{
    return &onlyuser;
}
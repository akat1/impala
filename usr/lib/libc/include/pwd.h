#ifndef __PWD_H
#define __PWD_H

#include <sys/types.h>

struct passwd {
    char    *pw_name;
    uid_t    pw_uid;
    gid_t    pw_gid;
    char    *pw_dir;
    char    *pw_shell;
};

struct passwd *getpwnam(const char *name);
struct passwd *getpwuid(uid_t uid);

#endif

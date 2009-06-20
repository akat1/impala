#ifndef __GRP_H__
#define __GRP_H__

struct group {
    char    *gr_name;
    char    *gr_passwd;
    gid_t    gr_gid;
    char   **gr_mem;
};



struct group *getgrnam(const char *name);
struct group *getgrgid(gid_t gid);




#endif

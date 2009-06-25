#include <grp.h>

struct group *
getgrnam(const char *name)
{
    struct group * res = malloc(sizeof(struct group));
    if(!res)
        return NULL;
    char buf[128];
    FILE *f = fopen("/etc/group", "r");
    if(!f)
        return NULL;
    while(fgets(buf, 128, f) != EOF) {
        char *sptr = buf;
        res->gr_name   = strsep(&sptr, ":");
        if(strcmp(res->gr_name, name))
            continue;
        res->gr_name = strdup(res->gr_name);
        res->gr_passwd = strdup(strsep(&sptr, ":"));
        res->gr_gid    = strdup(strsep(&sptr, ":"));
        char *tmp = sptr;
        int ucount = 0;
        while(*tmp) {
            if(*tmp == ',')
                ucount++;
            tmp++;
        }
        res->gr_mem = malloc(ucount*sizeof(char*));
        for(int i=0; i< ucount; i++)
            res->gr_mem[i] = strdup(strsep(&sptr, ","));
        return res;
    }
    return NULL;
}
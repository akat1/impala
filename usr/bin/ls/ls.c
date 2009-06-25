#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

const char *PROG = "ls";
#define error(fmt, a...) fprintf(stderr, "%s: " fmt "\n", PROG, ## a)

const char *
mode2str(mode_t m)
{
#define iss(bit, ch) ((m&bit)? ch : '-')
    static char out[11];
    out[0] = S_ISBLK(m)? 'b' :
                S_ISCHR(m)? 'c':
                S_ISDIR(m)? 'd':
                S_ISFIFO(m)? 'f':
                S_ISLNK(m)? 'l':
                S_ISREG(m)? '-':
                '?';
    out[1] = iss(S_IRUSR,  'r');
    out[2] = iss(S_IWUSR,  'w');
    out[3] = iss(S_IXUSR,  'x');
    out[4] = iss(S_IRGRP,  'r');
    out[5] = iss(S_IWGRP,  'w');
    out[6] = iss(S_IXGRP,  'x');
    out[7] = iss(S_IROTH,  'r');
    out[8] = iss(S_IWOTH,  'w');
    out[9] = iss(S_IXOTH,  'x');
    out[10] = 0;
#undef iss
    return out;
}


void
printverb(DIR *dir)
{
    struct dirent *dent;
    while ( (dent = readdir(dir)) ) {
        struct stat st;
        lstat(dent->d_name, &st);
        printf("%6s %7u %7u %10u !time! %s\n",
            mode2str(st.st_mode), st.st_uid, st.st_gid,
            st.st_size, /*time*/ dent->d_name);
    }
}


int
main(int argc, char **v)
{
// czekamy na getopt
    int verb = 1;
    DIR *dir = opendir(".");
    if (dir == NULL) {
        error("cannot open directory");
        return -1;
    }
    printverb(dir);
    closedir(dir);    
    return 0;
}

